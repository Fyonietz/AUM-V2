#pragma once
#include "../libs/Phoenix/mysql.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

class Database {
private:
  static std::unordered_map<std::string, std::string> loadConfigDb() {
    std::unordered_map<std::string, std::string> cfg;
    std::ifstream f("config/server.wpc");
    if (!f) {
      std::cerr << "Failed To Open Config File: config/server.wpc\n";
      return cfg;
    }
    std::string line;
    while (std::getline(f, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream in(line);
      std::string k, v;
      if (std::getline(in, k, '=') && std::getline(in, v))
        if (k.find("MySQL_") == 0 || k == "Database_Type") cfg[k] = v;
    }
    std::cout << "Database config loaded: " << cfg.size() << " entries\n";
    return cfg;
  }

  static std::string get(const std::unordered_map<std::string, std::string>& cfg,
                         const std::string& key, const std::string& def = "") {
    auto it = cfg.find(key);
    return (it != cfg.end()) ? it->second : def;
  }

public:
  static bool connect() {
    auto cfg = loadConfigDb();
    std::string host = get(cfg, "MySQL_Host", "localhost");
    std::string user = get(cfg, "MySQL_User", "root");
    std::string pass = get(cfg, "MySQL_Password", "");
    std::string db   = get(cfg, "MySQL_Database", "aum_phoenix");
    std::string port_str = get(cfg, "MySQL_Port", "3306");

    int port = 3306;
    try { port = std::stoi(port_str); } catch (...) {}

    std::cout << "Connecting to MySQL: " << user << "@" << host
              << ":" << port << "/" << db << std::endl;

    bool ok = MySQL_Connect(host, user, pass, db, port);
    std::cout << "MySQL connection: " << (ok ? "SUCCESS" : "FAILED") << std::endl;
    return ok;
  }

  static void close() { MySQL_Close(); }
  static std::string escape(const std::string& s) { return Escape(s); }
  static const MySQL& db() { return mysql; }
};

#define DB_OPEN() Database::connect()
#define DB_CONNECT() DB_OPEN()
#define DB_CLOSE() Database::close()
#define DB_ESCAPE(x) Database::escape(x)
#define db Database::db()

