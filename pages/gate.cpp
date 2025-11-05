
#include "../core/database.hpp"
#include "handler.hpp"
#include "json.hpp"
#include <exception>
#include <string>
using namespace Middleware;

route("/api/login", api_login) {

  if (!DB_CONNECT()) { // Fixed: DB_CONNECT not DB_OPEN
    return 500;
  }
  const struct mg_request_info *request_info = mg_get_request_info(connection);
  const char *method = request_info->request_method;
  std::cout << "login requested: " << &request_info << std::endl;
  std::cout << method << std::endl;
  // Handle preflight OPTIONS request
  if (strcmp(method, "OPTIONS") == 0) {
    mg_printf(connection, "HTTP/1.1 204 No Content\r\n");
    mg_printf(connection, "Access-Control-Allow-Origin: *\r\n");
    mg_printf(connection,
              "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    mg_printf(connection, "Access-Control-Allow-Headers: Content-Type, "
                          "Authorization, X-Requested-With\r\n");
    mg_printf(connection, "Access-Control-Max-Age: 86400\r\n\r\n");
    return 204; // No content response for preflight
  }
  std::cout << "login requested: " << request_info << std::endl;
  std::string req_body = Server.Read(connection);
  nlohmann::json post_as_json;
  try {
    post_as_json = nlohmann::json::parse(std::move(req_body));
  } catch (const std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 400, "Bad Request",
                           std::string("{\"error\":\"Invalid JSON: ") +
                               e.what() + "\"}");
  }

  Model<User> user_binder;
  user_binder.bind("nama", &User::nama)
      .bind("password", &User::password)
      .bind("role", &User::role_id);
  auto user_mapper = user_binder.parse_one(post_as_json);

  std::string recipe = user_mapper.nama + user_mapper.password + "masadepan";
  std::string token = Auth::tokenizer(recipe);

  Auth::Roles user;

  try {
    // Fixed: Use DB_ESCAPE instead of Escape
    std::string query = "token = " + DB_ESCAPE(token);

    nlohmann::json checker =
        db.SELECT("r.nama AS role, u.nama")
            .FROM("users u JOIN roles r ON u.role_id = r.id")
            .WHERE(query)
            .JSON();

    DB_CLOSE(); // Close connection after query

    if (checker.is_array() && !checker.empty()) {
      std::string role = checker[0]["role"];
      user_mapper.role_id = role;
    } else {
      return Server.Response(connection, 404, "User Not Found",
                             R"({"error":"User Not Found Please Register"})");
    }

    user = Auth::strToRole(user_mapper.role_id);
    std::cout << "DEBUG: Converted to enum: " << static_cast<int>(user)
              << std::endl;
    if (user == Auth::Roles::Unknown) {
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"Invalid Role"})");
    }

    std::string redirect_url;
    switch (user) {
    case Auth::Roles::Admin:
      redirect_url = "/dashboard/admin";
      break;
    case Auth::Roles::BK:
      redirect_url = "/dashboard/bk";
      break;
    case Auth::Roles::Siswa:
      redirect_url = "/dashboard/siswa";
      break;
    default:
      redirect_url = "/login";
      break;
    }

    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Set-Cookie: auth_token=%s; Path=/; HttpOnly; SameSite=Lax; "
              "Max-Age=3600\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n"
              "\r\n",
              token.c_str());

    std::string redirect_json =
        "{\"success\": true, \"redirect\": \"" + redirect_url + "\"}";
    mg_printf(connection, "%s", redirect_json.c_str());

    return 200;

  } catch (const std::exception &e) {
    std::cout << "Exception in database operation: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}

route("/api/ext/whoiam", whoiam) {
  if (!DB_CONNECT()) { // Fixed: DB_CONNECT not DB_OPEN
    return 500;
  }

  // Fixed: Use DB_ESCAPE instead of Escape
  std::string token = DB_ESCAPE(GetAuthToken(connection));
  std::string whereClause = "u.token = " + token;

  try {
    nlohmann::json query =
        db.SELECT("u.nama AS nama, k.nama AS kelas, r.nama AS role")
            .FROM("users u LEFT JOIN kelas k ON u.kelas_id = k.id "
                  "LEFT JOIN roles r ON u.role_id = r.id")
            .WHERE(whereClause)
            .JSON();

    Server.Response(connection, 200, "Ok", query.dump());
    DB_CLOSE();
    return 200;
  } catch (std::exception &e) {
    std::cout << "Exception in database operation: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}
route("/api/ext/stats", stats) {
  if (!DB_CONNECT()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           "Failed to open DB.\n");
  }

  try {
    // Most selected category
    nlohmann::json topCategory =
        db.SELECT("soal_masalah_kategori, COUNT(*) as total")
            .FROM("hasil")
            .GROUP_BY("soal_masalah_kategori")
            .ORDER_BY("total DESC")
            .LIMIT(1)
            .JSON();

    std::string mostSelectedCategory = "None";
    int mostSelectedCount = 0;

    if (!topCategory.empty() && topCategory.is_array()) {
      // Safe extraction for mostSelectedCategory
      if (topCategory[0]["soal_masalah_kategori"].is_string()) {
        mostSelectedCategory =
            topCategory[0]["soal_masalah_kategori"].get<std::string>();
      } else if (topCategory[0]["soal_masalah_kategori"].is_number()) {
        mostSelectedCategory =
            std::to_string(topCategory[0]["soal_masalah_kategori"].get<int>());
      }

      // Safe extraction for count
      if (topCategory[0]["total"].is_number_integer()) {
        mostSelectedCount = topCategory[0]["total"].get<int>();
      } else if (topCategory[0]["total"].is_string()) {
        mostSelectedCount =
            std::stoi(topCategory[0]["total"].get<std::string>());
      }
    }

    // Siswa role_id
    int role_id = 3;

    // Improved utility lambda to safely extract COUNT(*) as int
    auto safeCount = [](const nlohmann::json &j) -> int {
      if (!j.empty() && j.is_array() && j[0].contains("count")) {
        try {
          if (j[0]["count"].is_number_integer()) {
            return j[0]["count"].get<int>();
          } else if (j[0]["count"].is_string()) {
            return std::stoi(j[0]["count"].get<std::string>());
          } else if (j[0]["count"].is_number_float()) {
            return static_cast<int>(j[0]["count"].get<double>());
          }
        } catch (const std::exception &e) {
          std::cerr << "Error parsing count: " << e.what() << std::endl;
          return 0;
        }
      }
      return 0;
    };

    // Get totals safely using the improved lambda
    int totalAccounts =
        safeCount(db.SELECT("COUNT(*) as count")
                      .FROM("users")
                      .WHERE("role_id = " + std::to_string(role_id))
                      .JSON());

    int totalSubmissions = safeCount(
        db.SELECT("COUNT(DISTINCT user_id) as count").FROM("hasil").JSON());

    int totalCategory =
        safeCount(db.SELECT("COUNT(*) as count").FROM("bidang_masalah").JSON());

    int totalSubCategory =
        safeCount(db.SELECT("COUNT(*) as count").FROM("soal_masalah").JSON());

    // Build JSON response
    nlohmann::json jsonResponse = {
        {"most_selected_category", mostSelectedCategory},
        {"most_selected_count", mostSelectedCount},
        {"total_accounts", totalAccounts},
        {"total_submissions", totalSubmissions},
        {"total_category", totalCategory},
        {"total_sub_category", totalSubCategory}};

    Server.Response(connection, 200, "OK", jsonResponse.dump());
    DB_CLOSE();
    return 200;

  } catch (std::exception &e) {
    std::cerr << "Exception in stats API: " << e.what() << std::endl;
    DB_CLOSE();
    nlohmann::json errorJson = {{"error", e.what()}};
    return Server.Response(connection, 500, "Internal Server Error",
                           errorJson.dump());
  }
}
route("/login", login_pages) {
  Server.SSR("public/login.html", connection);
  return 200;
}

route("/logout", logout) {
  mg_printf(connection, "HTTP/1.1 302 Found\r\n"
                        "Location: /login\r\n"
                        "Set-Cookie: auth_token=; Path=/; Expires=Thu, 01 Jan "
                        "1970 00:00:00 GMT\r\n"
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n\r\n");
  return 302;
}
