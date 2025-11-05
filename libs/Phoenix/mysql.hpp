#pragma once
#include "../json.hpp"
#include <iostream>
#include <mysql/mysql.h>
#include <string>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

// Kept for backward compatibility
extern EXPORT MYSQL *g_mysql_conn;

// Modern thread-safe connection and helpers
EXPORT bool MySQL_Connect(const std::string &host = "localhost",
                          const std::string &user = "root",
                          const std::string &password = "",
                          const std::string &database = "aum_phoenix",
                          int port = 3306);
EXPORT void MySQL_Close();
EXPORT std::string Escape(const std::string &input);
EXPORT std::string RESCAPE(const std::string &input);
extern "C" struct EXPORT MySQL {
  enum QueryType {
    SELECT_Q,
    INSERT_Q,
    DELETE_Q,
    UPDATE_Q,
    NONE_Q
  } query_type = NONE_Q;

  std::string select_stmt;
  std::string from_stmt;
  std::string where_stmt;
  std::string group_by_stmt;
  std::string order_by_stmt;
  std::string limit_stmt;
  std::string insert_table;
  std::string insert_columns;
  std::string insert_values;
  std::string update_table;
  std::string set_stmt;

  // --- Fluent API ---
  MySQL SELECT(const std::string &stmt) const {
    MySQL copy = *this;
    copy.query_type = SELECT_Q;
    copy.select_stmt = stmt;
    return copy;
  }

  MySQL FROM(const std::string &stmt) const {
    MySQL copy = *this;
    copy.from_stmt = stmt;
    return copy;
  }

  MySQL WHERE(const std::string &stmt) const {
    MySQL copy = *this;
    copy.where_stmt = stmt;
    return copy;
  }

  MySQL GROUP_BY(const std::string &stmt) const {
    MySQL copy = *this;
    copy.group_by_stmt = stmt;
    return copy;
  }

  MySQL ORDER_BY(const std::string &stmt) const {
    MySQL copy = *this;
    copy.order_by_stmt = stmt;
    return copy;
  }

  MySQL LIMIT(int limit) const {
    MySQL copy = *this;
    copy.limit_stmt = std::to_string(limit);
    return copy;
  }

  MySQL INSERT(const std::string &table, const std::string &columns,
               const std::string &values) const {
    MySQL copy = *this;
    copy.query_type = INSERT_Q;
    copy.insert_table = table;
    copy.insert_columns = columns;
    copy.insert_values = values;
    return copy;
  }

  MySQL UPDATE(const std::string &table) const {
    MySQL copy = *this;
    copy.query_type = UPDATE_Q;
    copy.update_table = table;
    return copy;
  }

  MySQL SET(const std::string &stmt) const {
    MySQL copy = *this;
    copy.set_stmt = stmt;
    return copy;
  }

  MySQL DELETE(const std::string &table) const {
    MySQL copy = *this;
    copy.query_type = DELETE_Q;
    copy.from_stmt = table;
    return copy;
  }

  // --- Query Builder ---
  std::string build_query() const {
    switch (query_type) {
    case SELECT_Q: {
      std::string q = "SELECT " + select_stmt + " FROM " + from_stmt;
      if (!where_stmt.empty())
        q += " WHERE " + where_stmt;
      if (!group_by_stmt.empty())
        q += " GROUP BY " + group_by_stmt;
      if (!order_by_stmt.empty())
        q += " ORDER BY " + order_by_stmt;
      if (!limit_stmt.empty())
        q += " LIMIT " + limit_stmt;
      return q + ";";
    }
    case INSERT_Q:
      return "INSERT INTO " + insert_table + " " + insert_columns + " VALUES " +
             insert_values + ";";
    case DELETE_Q: {
      std::string q = "DELETE FROM " + from_stmt;
      if (!where_stmt.empty())
        q += " WHERE " + where_stmt;
      return q + ";";
    }
    case UPDATE_Q: {
      std::string q = "UPDATE " + update_table + " SET " + set_stmt;
      if (!where_stmt.empty())
        q += " WHERE " + where_stmt;
      return q + ";";
    }
    default:
      throw std::runtime_error("No query type specified");
    }
  }

  // --- Execute non-select ---
  void execute() const {
    if (query_type != INSERT_Q && query_type != DELETE_Q &&
        query_type != UPDATE_Q)
      throw std::runtime_error("execute() only for INSERT/DELETE/UPDATE");
    MYSQL *conn = nullptr;
    conn = MySQLConnection();
    std::string q = build_query();
    if (mysql_query(conn, q.c_str()) != 0)
      throw std::runtime_error("MySQL error: " +
                               std::string(mysql_error(conn)));
  }

  // --- Fetch as JSON ---
  nlohmann::json JSON() const {
    if (query_type != SELECT_Q)
      throw std::runtime_error("JSON() only for SELECT");
    MYSQL *conn = MySQLConnection();
    std::string q = build_query();
    if (mysql_query(conn, q.c_str()) != 0)
      throw std::runtime_error("MySQL error: " +
                               std::string(mysql_error(conn)));
    MYSQL_RES *res = mysql_store_result(conn);
    if (!res)
      return nlohmann::json::array();

    nlohmann::json arr = nlohmann::json::array();
    int nf = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
      nlohmann::json obj;
      for (int i = 0; i < nf; ++i) {
        std::string name = fields[i].name;
        obj[name] = row[i] ? row[i] : "";
      }
      arr.push_back(obj);
    }
    mysql_free_result(res);
    return arr;
  }

private:
  static MYSQL *MySQLConnection();
};

// Global instance
inline const MySQL mysql;
