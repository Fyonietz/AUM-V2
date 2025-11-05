#pragma once
#include "../json.hpp"
#include "engine.hpp"
#include <iostream>
#include <sqlite3.h>
#include <string>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

// Global SQLite DB handle
extern EXPORT sqlite3 *g_db;

EXPORT bool Sqlite_Open(const std::string &filename = "data.db");
EXPORT void Sqlite_Close();
// EXPORT std::string Escape(const std::string &input);

extern "C" struct EXPORT Sqlite3 {
  enum QueryType {
    SELECT_Q,
    INSERT_Q,
    DELETE_Q,
    UPDATE_Q,
    NONE_Q
  } query_type = NONE_Q;

  // ---- SELECT ----
  std::string select_stmt;
  std::string from_stmt;
  std::string where_stmt;
  std::string group_by_stmt;
  std::string order_by_stmt;
  std::string limit_stmt;

  // ---- INSERT ----
  std::string insert_table;
  std::string insert_columns;
  std::string insert_values;

  // ---- UPDATE ----
  std::string update_table;
  std::string set_stmt;

  // ---- SELECT Methods ----
  Sqlite3 SELECT(const std::string &select_stmt_f) const {
    Sqlite3 copy = *this;
    copy.query_type = SELECT_Q;
    copy.select_stmt = select_stmt_f;
    return copy;
  }

  Sqlite3 FROM(const std::string &from_stmt_f) const {
    Sqlite3 copy = *this;
    copy.from_stmt = from_stmt_f;
    return copy;
  }

  Sqlite3 WHERE(const std::string &where_stmt_f) const {
    Sqlite3 copy = *this;
    copy.where_stmt = where_stmt_f;
    return copy;
  }

  Sqlite3 GROUP_BY(const std::string &group_by_stmt_f) const {
    Sqlite3 copy = *this;
    copy.group_by_stmt = group_by_stmt_f;
    return copy;
  }

  Sqlite3 ORDER_BY(const std::string &order_by_stmt_f) const {
    Sqlite3 copy = *this;
    copy.order_by_stmt = order_by_stmt_f;
    return copy;
  }

  Sqlite3 LIMIT(int limit) const {
    Sqlite3 copy = *this;
    copy.limit_stmt = std::to_string(limit);
    return copy;
  }

  // ---- INSERT Methods ----
  Sqlite3 INSERT(const std::string &table, const std::string &columns,
                 const std::string &values) const {
    Sqlite3 copy = *this;
    copy.query_type = INSERT_Q;
    copy.insert_table = table;
    copy.insert_columns = columns;
    copy.insert_values = values;
    return copy;
  }

  // ---- UPDATE Methods ----
  Sqlite3 UPDATE(const std::string &table) const {
    Sqlite3 copy = *this;
    copy.query_type = UPDATE_Q;
    copy.update_table = table;
    return copy;
  }

  Sqlite3 SET(const std::string &set_stmt_f) const {
    Sqlite3 copy = *this;
    copy.set_stmt = set_stmt_f;
    return copy;
  }

  // ---- DELETE Methods ----
  Sqlite3 DELETE(const std::string &table) const {
    Sqlite3 copy = *this;
    copy.query_type = DELETE_Q;
    copy.from_stmt = table;
    return copy;
  }

  // ---- BUILD QUERY ----
  std::string build_query() const {
    switch (query_type) {
    case SELECT_Q: {
      std::string query = "SELECT " + select_stmt + " FROM " + from_stmt;
      if (!where_stmt.empty())
        query += " WHERE " + where_stmt;
      if (!group_by_stmt.empty())
        query += " GROUP BY " + group_by_stmt;
      if (!order_by_stmt.empty())
        query += " ORDER BY " + order_by_stmt;
      if (!limit_stmt.empty())
        query += " LIMIT " + limit_stmt;
      return query + ";";
    }
    case INSERT_Q:
      return "INSERT INTO " + insert_table + " " + insert_columns + " VALUES " +
             insert_values + ";";
    case DELETE_Q: {
      std::string query = "DELETE FROM " + from_stmt;
      if (!where_stmt.empty())
        query += " WHERE " + where_stmt;
      return query + ";";
    }
    case UPDATE_Q: {
      std::string query = "UPDATE " + update_table + " SET " + set_stmt;
      if (!where_stmt.empty())
        query += " WHERE " + where_stmt;
      return query + ";";
    }
    default:
      throw std::runtime_error("No query type specified");
    }
  }

  // ---- EXECUTE ----
  void execute() const {
    if (query_type != INSERT_Q && query_type != DELETE_Q &&
        query_type != UPDATE_Q) {
      throw std::runtime_error(
          "execute() only valid for INSERT, DELETE, or UPDATE queries");
    }
    if (!g_db)
      throw std::runtime_error("Database not opened");

    std::string query = build_query();
    char *errmsg = nullptr;
    int rc = sqlite3_exec(g_db, query.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
      std::string err_str = errmsg ? errmsg : "Unknown error";
      sqlite3_free(errmsg);
      throw std::runtime_error("SQL error: " + err_str);
    }
  }

  // ---- JSON SELECT ----
  nlohmann::json JSON() const {
    if (query_type != SELECT_Q)
      throw std::runtime_error("JSON() only valid for SELECT queries");
    if (!g_db)
      throw std::runtime_error("Database not opened");

    std::string query = build_query();
    sqlite3_stmt *stmt;
    nlohmann::json result = nlohmann::json::array();

    if (sqlite3_prepare_v2(g_db, query.c_str(), -1, &stmt, nullptr) !=
        SQLITE_OK) {
      throw std::runtime_error("Failed to prepare statement: " +
                               std::string(sqlite3_errmsg(g_db)));
    }

    int col_count = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      nlohmann::json row;
      for (int i = 0; i < col_count; ++i) {
        std::string col_name = sqlite3_column_name(stmt, i);
        int type = sqlite3_column_type(stmt, i);
        switch (type) {
        case SQLITE_INTEGER:
          row[col_name] = sqlite3_column_int(stmt, i);
          break;
        case SQLITE_FLOAT:
          row[col_name] = sqlite3_column_double(stmt, i);
          break;
        case SQLITE_TEXT:
          row[col_name] =
              reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
          break;
        case SQLITE_NULL:
          row[col_name] = nullptr;
          break;
        case SQLITE_BLOB:
          // Handle BLOB as string or skip
          row[col_name] = "[BLOB]";
          break;
        }
      }
      result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return result;
  }
};

// Global DSL instance
inline const Sqlite3 sqlite;
