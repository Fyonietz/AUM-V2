#include "sqlite3.hpp"
#include <iostream>
#include <sqlite3.h>

// Global SQLite database handle
sqlite3 *g_db = nullptr;

// Open the database globally once
bool Sqlite_Open(const std::string &filename) {
  if (g_db != nullptr) {
    std::cout << "Database already opened" << std::endl;
    return true;
  }

  int rc = sqlite3_open(filename.c_str(), &g_db);
  if (rc != SQLITE_OK) {
    std::cerr << "Cannot open database: " << sqlite3_errmsg(g_db) << std::endl;
    sqlite3_close(g_db);
    g_db = nullptr;
    return false;
  }

  // Enable foreign keys
  sqlite3_exec(g_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

  std::cout << "SQLite database opened successfully: " << filename << std::endl;
  return true;
}

// Close the database globally once
void Sqlite_Close() {
  if (g_db) {
    sqlite3_close(g_db);
    g_db = nullptr;
    std::cout << "SQLite database closed" << std::endl;
  }
}

// // Escape string for SQLite
// std::string Escape(const std::string &input) {
//   if (input.empty())
//     return "''";
//
//   std::string escaped = "'";
//   for (char c : input) {
//     if (c == '\'') {
//       escaped += "''"; // SQLite escape for single quote
//     } else {
//       escaped += c;
//     }
//   }
//   escaped += "'";
//   return escaped;
// }
