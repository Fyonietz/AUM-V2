
#include "../core/database.hpp"
#include "Phoenix/mysql.hpp"
#include "handler.hpp"
#include "json.hpp"
#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Middleware;

// ========================= UI =========================
route("/dashboard/siswa", siswa_dashboard) {
  auto authInfo =
      CheckAuthToken(connection, Auth::Roles::Siswa | Auth::Roles::Admin);
  if (!authInfo)
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");

  Server.static_serve("public/views/siswa/dashboard.html", connection);
  return 200;
}

route("/dashboard/siswa/aum", siswa_aum) {
  auto authInfo =
      CheckAuthToken(connection, Auth::Roles::Siswa | Auth::Roles::Admin);
  if (!authInfo)
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");

  Server.static_serve("public/views/siswa/aum.html", connection);
  return 200;
}

// ========================= API: /api/aum/serve =========================
route("/api/aum/serve", aum_serve) {
  if (!DB_OPEN())
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Failed to open DB"})");

  try {
    nlohmann::json query = db.SELECT("*").FROM("soal_masalah").JSON();
    DB_CLOSE();
    return Server.Response(connection, 200, "OK", query.dump());
  } catch (std::exception &e) {
    std::cerr << "[ERROR] /api/aum/serve: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}

// ========================= API: /api/aum/submit =========================

route("/api/aum/submit", aum_submit) {

  // 1. Authenticate user and check role
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Siswa);
  if (!authInfo) {
    DB_CLOSE();
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");
  }
  std::string token = GetAuthToken(connection);

  // 2. Open DB
  if (!DB_OPEN())
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Failed to open DB"})");

  try {
    // ====== GUARD CLAUSE: check if already submitted ======
    std::string query_clause = "token = " + DB_ESCAPE(token);
    auto submitted_check =
        db.SELECT("is_submited").FROM("users").WHERE(query_clause).JSON();
    int cond = std::stoi(submitted_check[0]["is_submited"].get<std::string>());
    if (cond == 0) {
      DB_CLOSE();
      Server.Response(
          connection, 409, "Conflict",
          R"({"success":false,"message":"You have already submitted"})");
      return 409;
    }

 // 3. Read POST data
    std::string post_data = Server.Read(connection);
    nlohmann::json post_json = nlohmann::json::parse(post_data);
    print(post_json.dump());

    if (!post_json.is_array()) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"Expected JSON array"})");
    }

    // 4. Load all users for lookup
    nlohmann::json users_json = db.SELECT("u.id AS user_id, u.nama, u.kelas_id")
                                    .FROM("users u")
                                    .WHERE(query_clause)
                                    .JSON();
    print(users_json.dump());
    
    std::unordered_map<std::string, int> user_map;
    for (auto &u : users_json) {
      // Convert string IDs to int
      int kelas_id = 0;
      int user_id = 0;

      if (u["kelas_id"].is_string()) {
        kelas_id = std::stoi(u["kelas_id"].get<std::string>());
      } else if (u["kelas_id"].is_number()) {
        kelas_id = u["kelas_id"].get<int>();
      }

      if (u["user_id"].is_string()) {
        user_id = std::stoi(u["user_id"].get<std::string>());
      } else if (u["user_id"].is_number()) {
        user_id = u["user_id"].get<int>();
      }

      std::string key = u.value("nama", "") + "|" + std::to_string(kelas_id);
      user_map[key] = user_id;
    }

    // 5. Load kelas mapping (nama -> id)
    nlohmann::json kelas_json = db.SELECT("id, nama").FROM("kelas").JSON();
    print(kelas_json.dump());
    std::unordered_map<std::string, int> kelas_map;
    for (auto &k : kelas_json) {
      // Safe parsing for kelas id
      int kelas_id = 0;
      if (k["id"].is_string()) {
        kelas_id = std::stoi(k["id"].get<std::string>());
      } else if (k["id"].is_number()) {
        kelas_id = k["id"].get<int>();
      }
      kelas_map[k.value("nama", "")] = kelas_id;
    }

    // 6. Prepare batch insert
    std::vector<std::string> values_list;
    for (const auto &item : post_json) {
      // Parse soal_masalah_id safely
      int soal_masalah_id = 0;
      if (item.contains("id")) {
        if (item["id"].is_number_integer()) {
          soal_masalah_id = item["id"].get<int>();
        } else if (item["id"].is_string()) {
          try {
            soal_masalah_id = std::stoi(item["id"].get<std::string>());
          } catch (...) {
            continue;
          }
        } else {
          continue;
        }
      } else {
        continue;
      }

      std::string nama = item.value("nama", "");
      std::string kelas_name = item.value("kelas", "");
      std::string soal_masalah_kategori = item.value("nama_bidang_masalah", "");

      // Convert kelas string -> kelas_id
      if (kelas_map.find(kelas_name) == kelas_map.end()) {
        std::cerr << "[Warning] Kelas not found: " << kelas_name << std::endl;
        continue;
      }
      int kelas_id = kelas_map[kelas_name];

      // Lookup user by nama + kelas_id
      std::string key = nama + "|" + std::to_string(kelas_id);
      if (user_map.find(key) == user_map.end()) {
        std::cerr << "[Warning] User not found: " << nama << " in kelas "
                  << kelas_name << std::endl;
        continue;
      }

      int target_user_id = user_map[key];

      // Prepare value string for insertion
      values_list.push_back("(" + std::to_string(target_user_id) + "," +
                            std::to_string(soal_masalah_id) + "," +
                            DB_ESCAPE(soal_masalah_kategori) + ")");
    }

    // 7. Insert all data into 'hasil'
    if (!values_list.empty()) {
      std::string all_values;
      for (size_t i = 0; i < values_list.size(); ++i) {
        all_values += values_list[i];
        if (i < values_list.size() - 1)
          all_values += ",";
      }
      db.INSERT("hasil", "(user_id, soal_masalah_id, soal_masalah_kategori)",
                all_values)
          .execute();
    }

    // 7. Mark user as submitted
    db.UPDATE("users").SET("is_submited=0").WHERE(query_clause).execute();

    DB_CLOSE();
    return Server.Response(
        connection, 200, "OK",
        R"({"success":true,"message":"Data inserted successfully"})");

  } catch (std::exception &e) {
    std::cerr << "[ERROR] /api/aum/submit: " << e.what() << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}
// ========================= API: /api/siswa/password/change
// =========================
route("/api/siswa/password/change", password_change) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Siswa);
  if (!authInfo)
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");

  if (!DB_OPEN())
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Failed to open DB"})");

  try {
    std::string post = Server.Read(connection);
    nlohmann::json post_as_json = nlohmann::json::parse(post);

    Model<User> user_binder;
    user_binder.bind("nama", &User::nama).bind("password_new", &User::password);

    auto user_mapper = user_binder.parse_one(post_as_json);

    std::string recipe = user_mapper.nama + user_mapper.password + "masadepan";
    std::string token = Auth::tokenizer(recipe);

    std::string query = "password=" + DB_ESCAPE(user_mapper.password) + "," +
                        "token=" + DB_ESCAPE(token);
    std::string clause = "nama=" + DB_ESCAPE(user_mapper.nama);

    db.UPDATE("users").SET(query).WHERE(clause).execute();

    DB_CLOSE();
    return Server.Response(connection, 200, "OK",
                           R"({"Response":"Update successfully"})");

  } catch (std::exception &e) {
    std::cerr << "[ERROR] /api/siswa/password/change: " << e.what()
              << std::endl;
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string("{\"error\":\"") + e.what() + "\"}");
  }
}
