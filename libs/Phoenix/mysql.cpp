
#include "mysql.hpp"
#include <mutex>
#include <thread>

// Thread-local MySQL connection
thread_local MYSQL* tls_mysql_conn = nullptr;

// Shared connection parameters
static std::string g_host = "localhost";
static std::string g_user = "root";
static std::string g_password = "";
static std::string g_database = "aum_phoenix";
static int g_port = 3306;

static std::once_flag lib_init_flag;

MYSQL* g_mysql_conn = nullptr; // legacy compatibility

// Internal helper
static MYSQL* get_thread_conn() {
  std::call_once(lib_init_flag, []() {
    mysql_library_init(0, nullptr, nullptr);
    std::cout << "[MySQL] Library initialized\n";
  });

  if (tls_mysql_conn) {
    if (mysql_ping(tls_mysql_conn) == 0)
      return tls_mysql_conn;
    mysql_close(tls_mysql_conn);
    tls_mysql_conn = nullptr;
  }

  tls_mysql_conn = mysql_init(nullptr);
  mysql_options(tls_mysql_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
  mysql_options(tls_mysql_conn, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4");

  if (!mysql_real_connect(tls_mysql_conn, g_host.c_str(), g_user.c_str(),
                          g_password.c_str(), g_database.c_str(),
                          g_port, nullptr, 0)) {
    std::cerr << "[MySQL] Connection failed: " << mysql_error(tls_mysql_conn)
              << std::endl;
    mysql_close(tls_mysql_conn);
    tls_mysql_conn = nullptr;
    return nullptr;
  }

  std::cout << "[MySQL] Connected thread " << std::this_thread::get_id()
            << " to " << g_database << "@" << g_host << ":" << g_port
            << std::endl;
  return tls_mysql_conn;
}

bool MySQL_Connect(const std::string& host, const std::string& user,
                   const std::string& password, const std::string& database,
                   int port) {
  g_host = host;
  g_user = user;
  g_password = password;
  g_database = database;
  g_port = port;
  return get_thread_conn() != nullptr;
}

void MySQL_Close() {
  if (tls_mysql_conn) {
    mysql_close(tls_mysql_conn);
    tls_mysql_conn = nullptr;
    std::cout << "[MySQL] Connection closed (thread "
              << std::this_thread::get_id() << ")\n";
  }
}

std::string Escape(const std::string& input) {
  MYSQL* conn = get_thread_conn();
  if (!conn) return "'" + input + "'";
  char* buf = new char[input.size() * 2 + 1];
  mysql_real_escape_string(conn, buf, input.c_str(), input.size());
  std::string out = "'" + std::string(buf) + "'";
  delete[] buf;
  return out;
}
EXPORT std::string RESCAPE(const std::string &input) {
  std::string temp;

  // Remove surrounding quotes if present
  for (char c : input) {
    if (c != '"' && c != '\'') { // remove " and '
      temp += c;
    }
  }

  // Optional: you can also validate that the remaining string is a number
  for (char c : temp) {
    if (!isdigit(c) && c != '-') { // allow negative numbers
      throw std::invalid_argument("Input contains non-digit characters");
    }
  }

  return temp;
}

// Static function implementation
MYSQL* MySQL::MySQLConnection() {
  MYSQL* conn = get_thread_conn();
  if (!conn)
    throw std::runtime_error("MySQL connection unavailable");
  return conn;
}

