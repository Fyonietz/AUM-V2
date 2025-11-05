#include "../core/database.hpp"
#include "../libs/Phoenix/mysql.hpp"
#include "handler.hpp"
#include <exception>
#include <string>
using namespace Middleware;
// External
int strToIdRole(const std::string &role) {
  static const std::unordered_map<std::string, int> lists = {
      {"Admin", 1}, {"BK", 2}, {"Siswa", 3}};

  auto it = lists.find(role);
  if (it != lists.end()) {
    return it->second;
  }
  return 0; // 0 for unknown role
}

route("/api/ext/roles", ext_roles) {
  if (!DB_OPEN())
    return Server.Response(connection, 500, "Internal Server Error",
                           "Failed to open DB.\n");
  try {
    auto authInfo =
        CheckAuthToken(connection, Auth::Roles::Admin | Auth::Roles::BK);
    if (!authInfo) {
      DB_CLOSE();
      return Server.Response(connection, 401, "Unauthorized",
                             R"({"error":"Unauthorized"})");
    }
    nlohmann::json roles_json =
        db.SELECT("id, nama").FROM("roles").ORDER_BY("id ASC").JSON();
    DB_CLOSE();
    Server.Response(connection, 200, "OK", roles_json.dump());
    return 200;
  } catch (std::exception &e) {
    std::cerr << "[ERROR] /api/admin/user/read: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}
// ====================== USER LIST ======================
route("/api/admin/user/read", akun) {
  if (!DB_OPEN())
    return Server.Response(connection, 500, "Internal Server Error",
                           "Failed to open DB.\n");

  try {
    auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
    if (!authInfo) {
      DB_CLOSE();
      return Server.Response(connection, 401, "Unauthorized",
                             R"({"error":"Unauthorized"})");
    }
    nlohmann::json akun_json =
        db.SELECT(R"(
                u.id,
                u.nama,
                k.nama AS kelas,
                r.nama AS role,
                u.password
            )")
            .FROM("users u JOIN kelas k ON u.kelas_id = k.id JOIN roles r ON "
                  "u.role_id = r.id")
            .ORDER_BY("u.id ASC")
            .JSON();

    DB_CLOSE();
    Server.Response(connection, 200, "OK", akun_json.dump());
    return 200;
  } catch (std::exception &e) {
    std::cerr << "[ERROR] /api/admin/user/read: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}

// ====================== CREATE USER ======================
route("/api/admin/user/create", admin_create_user) {

  try {
    auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
    if (!authInfo) {
      DB_CLOSE();
      return Server.Response(connection, 401, "Unauthorized",
                             R"({"error":"Unauthorized"})");
    }
    std::string post = Server.Read(connection);
    nlohmann::json post_as_json = nlohmann::json::parse(std::move(post));

    if (!DB_OPEN()) {
      return 500;
    }

    // Validate required fields
    if (!post_as_json.contains("nama") ||
        post_as_json["nama"].get<std::string>().empty()) {
      DB_CLOSE();
      Server.Response(connection, 400, "Bad Request",
                      R"({"error":"Nama is required"})");
      return 400;
    }

    if (!post_as_json.contains("password") ||
        post_as_json["password"].get<std::string>().empty()) {
      DB_CLOSE();
      Server.Response(connection, 400, "Bad Request",
                      R"({"error":"Password is required"})");
      return 400;
    }

    if (!post_as_json.contains("role") ||
        post_as_json["role"].get<std::string>().empty()) {
      DB_CLOSE();
      Server.Response(connection, 400, "Bad Request",
                      R"({"error":"Role is required"})");
      return 400;
    }

    std::string nama = post_as_json["nama"].get<std::string>();
    std::string password = post_as_json["password"].get<std::string>();
    std::string role = post_as_json["role"].get<std::string>();

    std::string recipe = nama + password + "masadepan";
    std::string token = Auth::tokenizer(recipe);
    int role_id = strToIdRole(role);

    // Handle kelas - different logic for BK vs other roles
    std::string kelas_id_value = "NULL";
    std::vector<int> kelas_ids; // For BK with multiple classes
    
    if (post_as_json.contains("kelas")) {
      if (post_as_json["kelas"].is_array()) {
        // Multiple classes (for BK) - only store in bk_class_duties
        for (const auto& kelas_item : post_as_json["kelas"]) {
          std::string kelas_name = kelas_item.get<std::string>();
          if (!kelas_name.empty()) {
            auto kelas_result = db.SELECT("id")
                                  .FROM("kelas")
                                  .WHERE("nama = " + DB_ESCAPE(kelas_name))
                                  .JSON();
            
            if (!kelas_result.empty()) {
              int found_kelas_id = 0;
              if (kelas_result[0]["id"].is_string()) {
                found_kelas_id = std::stoi(kelas_result[0]["id"].get<std::string>());
              } else {
                found_kelas_id = kelas_result[0]["id"].get<int>();
              }
              kelas_ids.push_back(found_kelas_id);
            }
          }
        }
        // For BK: keep kelas_id NULL in users table
        // Classes will be stored in bk_class_duties only
        
      } else if (post_as_json["kelas"].is_string()) {
        // Single class (for Siswa/Admin)
        std::string kelas = post_as_json["kelas"].get<std::string>();
        if (!kelas.empty()) {
          auto kelas_result = db.SELECT("id")
                                .FROM("kelas")
                                .WHERE("nama = " + DB_ESCAPE(kelas))
                                .JSON();

          if (!kelas_result.empty()) {
            int found_kelas_id = 0;
            if (kelas_result[0]["id"].is_string()) {
              found_kelas_id = std::stoi(kelas_result[0]["id"].get<std::string>());
            } else {
              found_kelas_id = kelas_result[0]["id"].get<int>();
            }
            
            // Only set kelas_id for non-BK users
            if (role_id != 2) {
              kelas_id_value = std::to_string(found_kelas_id);
            }
            kelas_ids.push_back(found_kelas_id);
          }
        }
      }
    }

    // Insert user (BK users will have NULL kelas_id)
    db.INSERT("users", "(nama, role_id, password, token, kelas_id)",
              "(" + DB_ESCAPE(nama) + ", " + std::to_string(role_id) + ", " +
                  DB_ESCAPE(password) + ", " + DB_ESCAPE(token) + ", " +
                  kelas_id_value + ")")
        .execute();

    // Get the newly created user ID
    auto new_user = db.SELECT("id")
                      .FROM("users")
                      .WHERE("nama = " + DB_ESCAPE(nama) + 
                             " AND password = " + DB_ESCAPE(password))
                      .ORDER_BY("id DESC")
                      .LIMIT(1)
                      .JSON();
    
    int user_id = 0;
    if (!new_user.empty()) {
      if (new_user[0]["id"].is_string()) {
        user_id = std::stoi(new_user[0]["id"].get<std::string>());
      } else {
        user_id = new_user[0]["id"].get<int>();
      }
    }

    // If BK role, mark as counselor and assign classes to bk_class_duties
    if (role_id == 2 && user_id > 0) {
      // Mark user as BK counselor
      db.UPDATE("users")
        .SET("is_bk_counselor = 1")
        .WHERE("id = " + std::to_string(user_id))
        .execute();
      
      // Insert all class assignments into bk_class_duties ONLY
      if (!kelas_ids.empty()) {
        for (int kelas_id : kelas_ids) {
          try {
            // Check if assignment already exists
            auto existing = db.SELECT("id")
                              .FROM("bk_class_duties")
                              .WHERE("user_id = " + std::to_string(user_id) + 
                                     " AND kelas_id = " + std::to_string(kelas_id))
                              .JSON();
            
            if (existing.empty()) {
              db.INSERT("bk_class_duties", "(user_id, kelas_id, is_active)",
                        "(" + std::to_string(user_id) + ", " + 
                        std::to_string(kelas_id) + ", 1)")
                  .execute();
            }
          } catch (std::exception &e) {
            std::cerr << "Warning: Failed to assign BK to kelas " 
                      << kelas_id << ": " << e.what() << std::endl;
          }
        }
      }
    }

    nlohmann::json response = {
      {"success", true},
      {"message", "User Created Successfully"},
      {"user_id", user_id}
    };

    Server.Response(connection, 200, "Ok", response.dump());
    DB_CLOSE();
    return 200;
  } catch (std::exception &e) {
    std::string error_msg = e.what();
    std::cout << "Exception in database operation: " << error_msg << std::endl;
    DB_CLOSE();

    // Handle duplicate user error
    if (error_msg.find("Duplicate entry") != std::string::npos) {
      return Server.Response(
          connection, 409, "Conflict",
          R"({"error":"User with this name already exists"})");
    }
    Server.Response(connection, 500, "Internal Server Error",
                    std::string("{\"error\":\"") + error_msg + "\"}");

    return 500;
  }
}

// ====================== DELETE USER ======================
route("/api/admin/user/delete", user_delete) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    if (!post_json.contains("nama") ||
        post_json["nama"].get<std::string>().empty()) {
      DB_CLOSE();
      Server.Response(connection, 400, "Bad Request",
                      R"({"error":"Nama kelas is required"})");
      return 400;
    }

    std::string nama = post_json["nama"].get<std::string>();

    auto existing =
        db.SELECT("id").FROM("users").WHERE("nama = " + Escape(nama)).JSON();

    if (existing.empty()) {
      DB_CLOSE();
      Server.Response(connection, 404, "Not Found",
                      R"({"error":"Kelas not found"})");

      return 404;
    }

    db.DELETE("users").WHERE("nama = " + Escape(nama)).execute();

    DB_CLOSE();
    Server.Response(
        connection, 200, "OK",
        R"({"success":true,"message":"Users deleted successfully"})");

    return 200;
  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}

// ====================== HELPER FUNCTIONS ======================

int get_or_create_kelas(const std::string &kelas_name) {
  auto result = db.SELECT("id")
                    .FROM("kelas")
                    .WHERE("nama = " + DB_ESCAPE(kelas_name))
                    .JSON();

  if (!result.empty() && result[0].contains("id")) {
    // Convert string to int safely if DB returns string
    if (result[0]["id"].is_number_integer())
      return result[0]["id"].get<int>();
    else if (result[0]["id"].is_string())
      return std::stoi(result[0]["id"].get<std::string>());
  }

  // Kelas doesn't exist → insert
  db.INSERT("kelas", "(nama)", "(" + DB_ESCAPE(kelas_name) + ")").execute();

  // Return the newly created ID
  auto new_result = db.SELECT("id")
                        .FROM("kelas")
                        .WHERE("nama = " + DB_ESCAPE(kelas_name))
                        .JSON();
  if (!new_result.empty()) {
    if (new_result[0]["id"].is_number_integer())
      return new_result[0]["id"].get<int>();
    else if (new_result[0]["id"].is_string())
      return std::stoi(new_result[0]["id"].get<std::string>());
  }

  throw std::runtime_error("Failed to get or create kelas ID");
}
// Password generator
std::string generate_4digit_password() {
  std::srand(std::time(nullptr));
  std::string password;
  for (int i = 0; i < 4; ++i) {
    password += std::to_string(std::rand() % 10);
  }
  return password;
}

// String trim
std::string trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r\f\v");
  if (start == std::string::npos)
    return "";
  size_t end = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(start, end - start + 1);
}
std::vector<std::pair<std::string, std::string>>
parse_frontend_csv_json(const std::string &content) {
  std::vector<std::pair<std::string, std::string>> result;

  try {
    auto j = nlohmann::json::parse(content);
    if (!j.contains("csv_data") || !j["csv_data"].is_array())
      return result;

    for (auto &item : j["csv_data"]) {
      if (!item.contains("nama") || !item.contains("kelas"))
        continue;

      std::string nama = trim(item["nama"].get<std::string>());
      std::string kelas = trim(item["kelas"].get<std::string>());
      if (!nama.empty() && !kelas.empty()) {
        result.emplace_back(nama, kelas);
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Failed to parse frontend CSV JSON payload: " << e.what()
              << std::endl;
  }

  return result;
} // Helper function to extract file from multipart data
std::string extract_file_from_multipart(const std::string &multipart_data) {
  // Look for CSV filename and content
  size_t filename_pos = multipart_data.find("filename=\"");
  if (filename_pos == std::string::npos) {
    return "";
  }

  // Find the start of file content (after headers)
  size_t content_start = multipart_data.find("\r\n\r\n", filename_pos);
  if (content_start == std::string::npos) {
    return "";
  }
  content_start += 4; // Move past \r\n\r\n

  // Find the end of file content (before boundary)
  size_t content_end = multipart_data.find("\r\n--", content_start);
  if (content_end == std::string::npos) {
    content_end = multipart_data.length();
  }

  return multipart_data.substr(content_start, content_end - content_start);
}
// ====================== IMPORT SISWA ======================

route("/api/admin/import-siswa", import_siswa) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");
  }

  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    mg_connection *mg_conn = reinterpret_cast<mg_connection *>(connection);

    // Read full body
    std::string body;
    char buf[4096];
    int read_len;
    while ((read_len = mg_read(mg_conn, buf, sizeof(buf))) > 0) {
      body.append(buf, read_len);
    }

    if (body.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"Empty request body"})");
    }

    // Extract CSV content from multipart
    std::string csv_content = extract_file_from_multipart(body);
    if (csv_content.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"No CSV file found in request"})");
    }

    // ====================== PARSE CSV/JSON ======================
    auto siswa_list = parse_frontend_csv_json(csv_content);

    std::vector<std::string> success_imports;
    std::vector<std::string> failed_imports;
    int siswa_role_id = 3;

    for (auto &[nama, kelas_name] : siswa_list) {
      std::string password = generate_4digit_password();

      std::string recipe = nama + password + "masadepan";
      std::string token = Middleware::Auth::tokenizer(recipe);

      try {
        // Get or create kelas safely
        int kelas_id = get_or_create_kelas(kelas_name);

        db.INSERT("users", "(nama, role_id, password, token, kelas_id)",
                  "(" + Escape(nama) + ", " + std::to_string(siswa_role_id) +
                      ", " + Escape(password) + ", " + Escape(token) + ", " +
                      std::to_string(kelas_id) + ")")
            .execute();

        success_imports.push_back(nama + " (" + kelas_name +
                                  ") - Password: " + password);

      } catch (std::exception &e) {
        std::string error_msg = e.what();
        if (error_msg.find("UNIQUE constraint failed") != std::string::npos)
          failed_imports.push_back(nama + ": User already exists");
        else
          failed_imports.push_back(nama + ": " + error_msg);
      }
    }

    nlohmann::json response = {{"success", true},
                               {"message", "Import completed"},
                               {"stats",
                                {{"total", siswa_list.size()},
                                 {"successful", success_imports.size()},
                                 {"failed", failed_imports.size()}}},
                               {"successful_imports", success_imports},
                               {"failed_imports", failed_imports}};

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", response.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    nlohmann::json errorJson = {{"error", e.what()}};
    return Server.Response(connection, 500, "Internal Server Error",
                           errorJson.dump());
  }
} // ====================== KELAS APIs ======================
route("/api/admin/kelas/read", kelas_read) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    auto result =
        db.SELECT("id, nama, jurusan").FROM("kelas").ORDER_BY("nama").JSON();

    DB_CLOSE();
    Server.Response(connection, 200, "OK", result.dump());
    return 200;
  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}

route("/api/admin/kelas/create", kelas_create) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    if (!post_json.contains("nama") ||
        post_json["nama"].get<std::string>().empty()) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"Nama kelas is required"})");
    }

    std::string nama = post_json["nama"].get<std::string>();
    std::string jurusan = post_json.value("jurusan", "");

    auto existing =
        db.SELECT("id").FROM("kelas").WHERE("nama = " + Escape(nama)).JSON();

    if (!existing.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 409, "Conflict",
                             R"({"error":"Kelas already exists"})");
    }

    std::string columns = "(nama";
    std::string values = "(" + Escape(nama);
    if (!jurusan.empty()) {
      columns += ", jurusan";
      values += ", " + Escape(jurusan);
    }
    columns += ")";
    values += ")";

    db.INSERT("kelas", columns, values).execute();

    auto new_kelas = db.SELECT("id, nama, jurusan")
                         .FROM("kelas")
                         .WHERE("nama = " + Escape(nama))
                         .JSON();

    DB_CLOSE();
    return Server.Response(connection, 201, "Created", new_kelas[0].dump());
  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}

route("/api/admin/kelas/delete", kelas_delete) {
  // --- Check authorization ---
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  // --- Open database ---
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // --- Read JSON input ---
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    // --- Validate input ---
    if (!post_json.contains("nama") ||
        post_json["nama"].get<std::string>().empty()) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"Nama kelas is required"})");
    }

    std::string nama = post_json["nama"].get<std::string>();

    // --- Check if the class exists ---
    auto existing = db.SELECT("id, nama, jurusan")
                        .FROM("kelas")
                        .WHERE("nama = " + Escape(nama))
                        .JSON();

    if (existing.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"Kelas not found"})");
    }

    // --- Perform deletion ---
    db.DELETE("kelas").WHERE("nama = " + Escape(nama)).execute();

    DB_CLOSE();
    return Server.Response(connection, 200, "OK",
                           R"({"message":"Kelas deleted successfully"})");

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}
// BK Control

route("/api/admin/users/mark-bk", mark_user_bk) {
  // --- Check authentication ---
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  // --- Open MariaDB connection ---
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // --- Read and parse request JSON ---
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    // --- Validate required fields ---
    if (!post_json.contains("user_id") || !post_json.contains("kelas_id")) {
      DB_CLOSE();
      return Server.Response(
          connection, 400, "Bad Request",
          R"({"error":"user_id and kelas_id are required"})");
    }

    int user_id = post_json["user_id"].get<int>();
    int kelas_id = post_json["kelas_id"].get<int>();
    bool is_active = post_json.value("is_active", true);

    // --- Check if user exists ---
    auto user_exists = db.SELECT("id")
                           .FROM("users")
                           .WHERE("id = " + std::to_string(user_id))
                           .JSON();

    if (user_exists.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"User not found"})");
    }

    // --- Check if kelas exists ---
    auto kelas_exists = db.SELECT("id")
                            .FROM("kelas")
                            .WHERE("id = " + std::to_string(kelas_id))
                            .JSON();

    if (kelas_exists.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"Kelas not found"})");
    }

    // --- Check if user already assigned to the class ---
    auto existing_duty =
        db.SELECT("id")
            .FROM("bk_class_duties")
            .WHERE("user_id = " + std::to_string(user_id) +
                   " AND kelas_id = " + std::to_string(kelas_id))
            .JSON();

    if (existing_duty.empty()) {
      // Insert new duty
      db.INSERT("bk_class_duties", "(user_id, kelas_id, is_active)",
                "(" + std::to_string(user_id) + ", " +
                    std::to_string(kelas_id) + ", " +
                    std::to_string(is_active ? 1 : 0) + ")")
          .execute();
    } else {
      // Update existing assignment
      db.UPDATE("bk_class_duties")
          .SET("is_active = " + std::to_string(is_active ? 1 : 0))
          .WHERE("user_id = " + std::to_string(user_id) +
                 " AND kelas_id = " + std::to_string(kelas_id))
          .execute();
    }

    // --- Fetch updated record ---
    auto updated = db.SELECT("id, user_id, kelas_id, is_active, assigned_at")
                       .FROM("bk_class_duties")
                       .WHERE("user_id = " + std::to_string(user_id) +
                              " AND kelas_id = " + std::to_string(kelas_id))
                       .JSON();

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", updated[0].dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}
route("/api/admin/bk-duties/assign", bk_duties_assign) {
  // --- Authorization check ---
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  // --- Open DB connection ---
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // --- Parse JSON body ---
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    // --- Validate inputs ---
    if (!post_json.contains("user_id") || !post_json.contains("kelas_ids") ||
        !post_json["kelas_ids"].is_array()) {
      DB_CLOSE();
      return Server.Response(
          connection, 400, "Bad Request",
          R"({"error":"user_id and kelas_ids array are required"})");
    }

    int user_id = post_json["user_id"].get<int>();
    auto kelas_ids = post_json["kelas_ids"];

    // --- Check if user exists and is BK counselor ---
    auto user_check = db.SELECT("id, nama, is_bk_counselor")
                          .FROM("users")
                          .WHERE("id = " + std::to_string(user_id))
                          .JSON();

    if (user_check.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"User not found"})");
    }

    // --- FIX: Safe boolean parsing for is_bk_counselor ---
    bool is_bk_counselor = false;
    if (user_check[0].contains("is_bk_counselor")) {
      if (user_check[0]["is_bk_counselor"].is_boolean()) {
        is_bk_counselor = user_check[0]["is_bk_counselor"].get<bool>();
      } else if (user_check[0]["is_bk_counselor"].is_string()) {
        std::string bk_val =
            user_check[0]["is_bk_counselor"].get<std::string>();
        is_bk_counselor = (bk_val == "1" || bk_val == "true");
      } else if (user_check[0]["is_bk_counselor"].is_number()) {
        is_bk_counselor = user_check[0]["is_bk_counselor"].get<int>() == 1;
      }
    }

    if (!is_bk_counselor) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"User is not a BK counselor"})");
    }

    // --- Result trackers ---
    std::vector<std::string> success_assignments;
    std::vector<std::string> failed_assignments;

    // --- Process each kelas_id ---
    for (const auto &kelas_id_json : kelas_ids) {
      try {
        int kelas_id = 0;

        // Safe parsing for kelas_id
        if (kelas_id_json.is_number()) {
          kelas_id = kelas_id_json.get<int>();
        } else if (kelas_id_json.is_string()) {
          try {
            kelas_id = std::stoi(kelas_id_json.get<std::string>());
          } catch (...) {
            failed_assignments.push_back("Invalid kelas ID format: " +
                                         kelas_id_json.dump());
            continue;
          }
        } else {
          failed_assignments.push_back("Invalid kelas ID type: " +
                                       kelas_id_json.dump());
          continue;
        }

        // --- Check if kelas exists ---
        auto kelas_exists = db.SELECT("id, nama")
                                .FROM("kelas")
                                .WHERE("id = " + std::to_string(kelas_id))
                                .JSON();

        if (kelas_exists.empty()) {
          failed_assignments.push_back("Kelas ID " + std::to_string(kelas_id) +
                                       " not found");
          continue;
        }

        std::string kelas_nama = "";
        if (kelas_exists[0]["nama"].is_string()) {
          kelas_nama = kelas_exists[0]["nama"].get<std::string>();
        } else if (kelas_exists[0]["nama"].is_number()) {
          kelas_nama = std::to_string(kelas_exists[0]["nama"].get<int>());
        }

        // --- Check if assignment already exists ---
        auto existing_assignment =
            db.SELECT("id")
                .FROM("bk_class_duties")
                .WHERE("user_id = " + std::to_string(user_id) +
                       " AND kelas_id = " + std::to_string(kelas_id))
                .JSON();

        if (existing_assignment.empty()) {
          // Insert new assignment
          db.INSERT("bk_class_duties", "(user_id, kelas_id)",
                    "(" + std::to_string(user_id) + ", " +
                        std::to_string(kelas_id) + ")")
              .execute();
        } else {
          // Update existing assignment (reactivate)
          db.UPDATE("bk_class_duties")
              .SET("is_active = 1")
              .WHERE("user_id = " + std::to_string(user_id) +
                     " AND kelas_id = " + std::to_string(kelas_id))
              .execute();
        }

        success_assignments.push_back(kelas_nama);
      } catch (std::exception &e) {
        std::string error_msg = "Kelas ID processing error: ";
        error_msg += e.what();
        failed_assignments.push_back(error_msg);
      }
    }

    DB_CLOSE();

    // --- Prepare response JSON ---
    nlohmann::json response = {{"success", true},
                               {"message", "Assignments completed"},
                               {"user_id", user_id},
                               {"successful_assignments", success_assignments},
                               {"failed_assignments", failed_assignments}};

    return Server.Response(connection, 200, "OK", response.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}
route("/api/admin/bk-duties/remove", bk_duties_remove) {
  // --- Authorization ---
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  // --- Open DB connection ---
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // --- Read and parse JSON ---
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);

    // --- Validate input ---
    if (!post_json.contains("user_id") || !post_json.contains("kelas_ids") ||
        !post_json["kelas_ids"].is_array()) {
      DB_CLOSE();
      return Server.Response(
          connection, 400, "Bad Request",
          R"({"error":"user_id and kelas_ids array are required"})");
    }

    int user_id = post_json["user_id"].get<int>();
    auto kelas_ids = post_json["kelas_ids"];

    std::vector<std::string> success_removals;
    std::vector<std::string> failed_removals;

    // --- Process each kelas_id ---
    for (const auto &kelas_id_json : kelas_ids) {
      try {
        int kelas_id = kelas_id_json.get<int>();

        // --- Check if assignment exists (JOIN to get class name) ---
        auto assignment_exists =
            db.SELECT("bd.id, k.nama AS kelas_nama")
                .FROM("bk_class_duties bd JOIN kelas k ON bd.kelas_id = k.id")
                .WHERE("bd.user_id = " + std::to_string(user_id) +
                       " AND bd.kelas_id = " + std::to_string(kelas_id))
                .JSON();

        if (assignment_exists.empty()) {
          failed_removals.push_back("Assignment not found for kelas ID " +
                                    std::to_string(kelas_id));
          continue;
        }

        std::string kelas_nama =
            assignment_exists[0]["kelas_nama"].get<std::string>();

        // --- Option A: Soft delete (recommended) ---
        db.UPDATE("bk_class_duties")
            .SET("is_active = 0")
            .WHERE("user_id = " + std::to_string(user_id) +
                   " AND kelas_id = " + std::to_string(kelas_id))
            .execute();

        // --- Option B: Hard delete (uncomment if you want full deletion) ---
        // db.DELETE_FROM("bk_class_duties")
        //     .WHERE("user_id = " + std::to_string(user_id) +
        //            " AND kelas_id = " + std::to_string(kelas_id))
        //     .execute();

        success_removals.push_back(kelas_nama);

      } catch (std::exception &e) {
        failed_removals.push_back("Kelas ID " +
                                  std::to_string(kelas_id_json.get<int>()) +
                                  ": " + e.what());
      }
    }

    DB_CLOSE();

    // --- Response JSON ---
    nlohmann::json response = {{"success", true},
                               {"message", "Removals completed"},
                               {"user_id", user_id},
                               {"successful_removals", success_removals},
                               {"failed_removals", failed_removals}};

    return Server.Response(connection, 200, "OK", response.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}

route("/api/admin/bk-duties/list", bk_duties_list) {
  // --- Authorization check ---
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }

  // --- Open MariaDB connection ---
  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // --- Query BK counselors and their classes ---
    auto result = db.SELECT("u.id AS user_id, "
                            "u.nama AS counselor_name, "
                            "u.bk_specialization, "
                            "GROUP_CONCAT(k.id) AS kelas_ids, "
                            "GROUP_CONCAT(k.nama) AS kelas_names")
                      .FROM("users u "
                            "LEFT JOIN bk_class_duties bd ON u.id = bd.user_id "
                            "AND bd.is_active = 1 "
                            "LEFT JOIN kelas k ON bd.kelas_id = k.id")
                      .WHERE("u.is_bk_counselor = 1")
                      .GROUP_BY("u.id")
                      .JSON();

    nlohmann::json counselors = nlohmann::json::array();

    // --- Helper lambda for string splitting ---
    auto split = [](const std::string &str,
                    char delimiter) -> std::vector<std::string> {
      std::vector<std::string> tokens;
      std::string token;
      std::istringstream tokenStream(str);
      while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
      }
      return tokens;
    };

    // --- Process rows into structured JSON ---
    for (auto &row : result) {
      nlohmann::json counselor = {
          {"user_id", row["user_id"]},
          {"counselor_name", row["counselor_name"]},
          {"bk_specialization", row.value("bk_specialization", "")},
          {"assigned_classes", nlohmann::json::array()}};

      if (!row["kelas_ids"].is_null() && !row["kelas_names"].is_null()) {
        std::string kelas_ids_str = row["kelas_ids"].get<std::string>();
        std::string kelas_names_str = row["kelas_names"].get<std::string>();

        auto kelas_ids = split(kelas_ids_str, ',');
        auto kelas_names = split(kelas_names_str, ',');

        for (size_t i = 0; i < kelas_ids.size() && i < kelas_names.size();
             i++) {
          if (!kelas_ids[i].empty() && !kelas_names[i].empty()) {
            counselor["assigned_classes"].push_back(
                {{"id", std::stoi(kelas_ids[i])}, {"nama", kelas_names[i]}});
          }
        }
      }

      counselors.push_back(counselor);
    }

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", counselors.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}
// ====================== DASHBOARD ======================
route("/dashboard/admin", admin_dashboard) {
  auto authInfo = CheckAuthToken(
      connection, Auth::Roles::Admin); // Use Admin role, not None

  if (!authInfo) {
    DB_CLOSE(); // Close connection inside the if block
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");
  }

  // Only execute this if authentication succeeded
  Server.static_serve("public/views/admin/dashboard.html", connection);
  DB_CLOSE(); // Close connection after serving the page
  return 200;
}
