
#include "../core/database.hpp"
#include "handler.hpp"
#include "models/middleware.hpp"
#include <exception>
#include <iostream>

using namespace Middleware;

// ========================= API: /api/bk/dashboard =========================
route("/api/bk/dashboard", bk_dashboard) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::BK);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }
  std::string token = GetAuthToken(connection);

  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // Get BK counselor info
    std::string user_query = "token = " + DB_ESCAPE(token);
    auto bk_info = db.SELECT("id, nama, bk_specialization")
                       .FROM("users")
                       .WHERE(user_query)
                       .JSON();

    if (bk_info.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"BK counselor not found"})");
    }

    int bk_user_id = std::stoi(bk_info[0]["id"].get<std::string>());
    std::string bk_name = bk_info[0]["nama"].get<std::string>();
    std::string specialization = bk_info[0].value("bk_specialization", "");

    // Get assigned classes for this BK counselor
    auto assigned_classes =
        db.SELECT("kelas_id")
            .FROM("bk_class_duties")
            .WHERE("user_id = " + std::to_string(bk_user_id) +
                   " AND is_active = 1")
            .JSON();

    // If no classes assigned, return empty results
    if (assigned_classes.empty()) {
      nlohmann::json response = {
          {"bk_counselor",
           {{"id", bk_user_id},
            {"nama", bk_name},
            {"specialization", specialization}}},
          {"assigned_classes", nlohmann::json::array()},
          {"class_statistics", nlohmann::json::array()},
          {"total_students", 0},
          {"total_submissions", 0},
          {"total_not_submitted", 0},
          {"problem_categories", nlohmann::json::array()}};

      DB_CLOSE();
      return Server.Response(connection, 200, "OK", response.dump());
    }

    // Extract class IDs for queries
    std::vector<int> class_ids;
    for (const auto &cls : assigned_classes) {
      class_ids.push_back(std::stoi(cls["kelas_id"].get<std::string>()));
    }

    std::string class_ids_str;
    for (size_t i = 0; i < class_ids.size(); ++i) {
      class_ids_str += std::to_string(class_ids[i]);
      if (i < class_ids.size() - 1)
        class_ids_str += ",";
    }

    // Get class details
    auto class_details = db.SELECT("id, nama, jurusan")
                             .FROM("kelas")
                             .WHERE("id IN (" + class_ids_str + ")")
                             .JSON();

    // Get total students in assigned classes
    auto total_students_json = db.SELECT("COUNT(*) as total")
                                   .FROM("users")
                                   .WHERE("kelas_id IN (" + class_ids_str +
                                          ") AND role_id = 3") // Siswa role
                                   .JSON();

    int total_students = 0;
    if (!total_students_json.empty()) {
      total_students =
          std::stoi(total_students_json[0]["total"].get<std::string>());
    }

       auto submitted_students_json =
        db.SELECT("COUNT(*) as total")
            .FROM("users")
            .WHERE("kelas_id IN (" + class_ids_str +
                   ") AND role_id = 3 AND is_submited = 0")
            .JSON();

    int total_submissions = 0;
    if (!submitted_students_json.empty()) {
      total_submissions =
          std::stoi(submitted_students_json[0]["total"].get<std::string>());
    }

    int total_not_submitted = total_students - total_submissions;

    // Problem categories distribution
    auto problem_categories =
        db.SELECT("soal_masalah_kategori, COUNT(*) as count")
            .FROM("hasil")
            .WHERE("user_id IN (SELECT id FROM users WHERE kelas_id IN (" +
                   class_ids_str + ") AND role_id = 3)")
            .GROUP_BY("soal_masalah_kategori")
            .ORDER_BY("count DESC")
            .JSON();

    // Statistics per class
    nlohmann::json class_statistics = nlohmann::json::array();
    for (const auto &cls : class_details) {
      int kelas_id = std::stoi(cls["id"].get<std::string>());
      std::string kelas_nama = cls["nama"].get<std::string>();

      auto class_students_json =
          db.SELECT("COUNT(*) as total")
              .FROM("users")
              .WHERE("kelas_id = " + std::to_string(kelas_id) +
                     " AND role_id = 3")
              .JSON();

      int class_students = 0;
      if (!class_students_json.empty()) {
        class_students =
            std::stoi(class_students_json[0]["total"].get<std::string>());
      }

      auto class_submitted_json =
          db.SELECT("COUNT(*) as total")
              .FROM("users")
              .WHERE("kelas_id = " + std::to_string(kelas_id) +
                     " AND role_id = 3 AND is_submited = 0")
              .JSON();

      int class_submissions = 0;
      if (!class_submitted_json.empty()) {
        class_submissions =
            std::stoi(class_submitted_json[0]["total"].get<std::string>());
      }

      int class_not_submitted = class_students - class_submissions;

      class_statistics.push_back(
          {{"kelas_id", kelas_id},
           {"kelas_nama", kelas_nama},
           {"jurusan", cls.value("jurusan", "")},
           {"total_students", class_students},
           {"total_submissions", class_submissions},
           {"total_not_submitted", class_not_submitted},
           {"completion_rate",
            class_students > 0 ? (class_submissions * 100.0 / class_students)
                               : 0}});
    }

    // Build final response
    nlohmann::json response = {{"bk_counselor",
                                {{"id", bk_user_id},
                                 {"nama", bk_name},
                                 {"specialization", specialization}}},
                               {"assigned_classes", class_details},
                               {"class_statistics", class_statistics},
                               {"total_students", total_students},
                               {"total_submissions", total_submissions},
                               {"total_not_submitted", total_not_submitted},
                               {"problem_categories", problem_categories}};

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", response.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}

// ========================= API: /api/bk/student-results
// =========================
route("/api/bk/student-results", bk_student_results) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::BK);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }
  std::string token = GetAuthToken(connection);

  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // Get BK counselor's assigned classes
    std::string user_query = "token = " + DB_ESCAPE(token);
    auto bk_info = db.SELECT("id").FROM("users").WHERE(user_query).JSON();

    if (bk_info.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"BK counselor not found"})");
    }

    int bk_user_id = std::stoi(bk_info[0]["id"].get<std::string>());

    // Get assigned class IDs
    auto assigned_classes =
        db.SELECT("kelas_id")
            .FROM("bk_class_duties")
            .WHERE("user_id = " + std::to_string(bk_user_id) +
                   " AND is_active = 1")
            .JSON();

    if (assigned_classes.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 200, "OK", "[]");
    }

    // Build class IDs string
    std::string class_ids_str;
    for (size_t i = 0; i < assigned_classes.size(); ++i) {
      class_ids_str += assigned_classes[i]["kelas_id"].get<std::string>();
      if (i < assigned_classes.size() - 1)
        class_ids_str += ",";
    }

    // Get students from assigned classes
    auto students =
        db.SELECT("id, nama, kelas_id, is_submited")
            .FROM("users")
            .WHERE("kelas_id IN (" + class_ids_str + ") AND role_id = 3")
            .ORDER_BY("kelas_id, nama")
            .JSON();

    // Get kelas names
    auto kelas_names = db.SELECT("id, nama, jurusan")
                           .FROM("kelas")
                           .WHERE("id IN (" + class_ids_str + ")")
                           .JSON();

    // Create kelas mapping
    std::unordered_map<int, std::string> kelas_map;
    std::unordered_map<int, std::string> jurusan_map;
    for (const auto &k : kelas_names) {
      int k_id = std::stoi(k["id"].get<std::string>());
      kelas_map[k_id] = k["nama"].get<std::string>();
      jurusan_map[k_id] = k.value("jurusan", "");
    }

    // Get problem counts for each student
    nlohmann::json student_results = nlohmann::json::array();

    for (const auto &student : students) {
      int user_id = std::stoi(student["id"].get<std::string>());
      int kelas_id = std::stoi(student["kelas_id"].get<std::string>());

      // Get problem count and categories for this student
      auto student_problems =
          db.SELECT("COUNT(*) as problem_count, GROUP_CONCAT(DISTINCT "
                    "soal_masalah_kategori) as categories")
              .FROM("hasil")
              .WHERE("user_id = " + std::to_string(user_id))
              .JSON();

      int problem_count = 0;
      std::string categories = "";

      if (!student_problems.empty()) {
        problem_count =
            std::stoi(student_problems[0]["problem_count"].get<std::string>());
        categories = student_problems[0].value("categories", "");
      }


      bool has_submitted =
          (std::stoi(student["is_submited"].get<std::string>()) == 0);

      student_results.push_back(
          {{"user_id", user_id},
           {"nama", student["nama"].get<std::string>()},
           {"kelas", kelas_map[kelas_id]},
           {"jurusan", jurusan_map[kelas_id]},
           {"is_submited", student["is_submited"]},
           {"has_submitted", has_submitted}, // Add clear boolean field
           {"problem_count", problem_count},
           {"categories", categories}});
    }

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", student_results.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}
// ========================= API: /api/bk/detailed-results =========================
route("/api/bk/detailed-results", bk_detailed_results) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::BK);
  if (!authInfo) {
    DB_CLOSE();
    return Server.Response(connection, 401, "Unauthorized",
                           R"({"error":"Unauthorized"})");
  }
  std::string token = GetAuthToken(connection);

  if (!DB_OPEN()) {
    return Server.Response(connection, 500, "Internal Server Error",
                           R"({"error":"Database connection failed"})");
  }

  try {
    // Manual query string parsing for CivetWeb
    const struct mg_request_info *req_info = mg_get_request_info(connection);
    std::string query_string = req_info->query_string ? req_info->query_string : "";
    
    // Parse student_id from query string
    int student_id = 0;
    if (!query_string.empty()) {
      size_t pos = query_string.find("student_id=");
      if (pos != std::string::npos) {
        std::string student_id_str = query_string.substr(pos + 11);
        size_t end_pos = student_id_str.find('&');
        if (end_pos != std::string::npos) {
          student_id_str = student_id_str.substr(0, end_pos);
        }
        student_id = std::stoi(student_id_str);
      }
    }

    if (student_id == 0) {
      DB_CLOSE();
      return Server.Response(connection, 400, "Bad Request",
                             R"({"error":"student_id parameter is required"})");
    }

    // Verify student is in BK's assigned classes
    std::string user_query = "token = " + DB_ESCAPE(token);
    auto bk_info = db.SELECT("id")
                     .FROM("users")
                     .WHERE(user_query)
                     .JSON();

    if (bk_info.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 404, "Not Found",
                             R"({"error":"BK counselor not found"})");
    }

    int bk_user_id = std::stoi(bk_info[0]["id"].get<std::string>());

    // Get BK's assigned classes
    auto assigned_classes = db.SELECT("kelas_id")
                              .FROM("bk_class_duties")
                              .WHERE("user_id = " + std::to_string(bk_user_id) + 
                                     " AND is_active = 1")
                              .JSON();

    if (assigned_classes.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 403, "Forbidden",
                             R"({"error":"No classes assigned to this BK counselor"})");
    }

    // Build class IDs string
    std::string class_ids_str;
    for (size_t i = 0; i < assigned_classes.size(); ++i) {
      class_ids_str += assigned_classes[i]["kelas_id"].get<std::string>();
      if (i < assigned_classes.size() - 1) class_ids_str += ",";
    }

    // Check if student is in BK's assigned classes
    auto student_check = db.SELECT("id, nama, kelas_id")
                           .FROM("users")
                           .WHERE("id = " + std::to_string(student_id) + 
                                  " AND kelas_id IN (" + class_ids_str + ")")
                           .JSON();

    if (student_check.empty()) {
      DB_CLOSE();
      return Server.Response(connection, 403, "Forbidden",
                             R"({"error":"Access denied to this student's data"})");
    }

    // Get student class info
    int kelas_id = std::stoi(student_check[0]["kelas_id"].get<std::string>());
    auto kelas_info = db.SELECT("nama, jurusan")
                        .FROM("kelas")
                        .WHERE("id = " + std::to_string(kelas_id))
                        .JSON();

    std::string kelas_nama = "";
    std::string jurusan = "";
    if (!kelas_info.empty()) {
      kelas_nama = kelas_info[0]["nama"].get<std::string>();
      jurusan = kelas_info[0].value("jurusan", "");
    }

    // Get problem categories and counts for the student
    auto categories = db.SELECT("soal_masalah_kategori as category, COUNT(*) as count")
                        .FROM("hasil")
                        .WHERE("user_id = " + std::to_string(student_id))
                        .GROUP_BY("soal_masalah_kategori")
                        .ORDER_BY("count DESC")
                        .JSON();

    // Get all problem selections for this student
    auto all_problems = db.SELECT("soal_masalah_id, soal_masalah_kategori")
                         .FROM("hasil")
                         .WHERE("user_id = " + std::to_string(student_id))
                         .JSON();

    // Get all soal_masalah details
    std::vector<int> problem_ids;
    for (const auto& problem : all_problems) {
      problem_ids.push_back(std::stoi(problem["soal_masalah_id"].get<std::string>()));
    }

    std::string problem_ids_str;
    for (size_t i = 0; i < problem_ids.size(); ++i) {
      problem_ids_str += std::to_string(problem_ids[i]);
      if (i < problem_ids.size() - 1) problem_ids_str += ",";
    }

    // Get problem names
    std::unordered_map<int, std::string> problem_names;
    if (!problem_ids_str.empty()) {
      auto problem_details = db.SELECT("id, nama_soal_masalah")
                             .FROM("soal_masalah")
                             .WHERE("id IN (" + problem_ids_str + ")")
                             .JSON();

      for (const auto& problem : problem_details) {
        int prob_id = std::stoi(problem["id"].get<std::string>());
        problem_names[prob_id] = problem["nama_soal_masalah"].get<std::string>();
      }
    }

    // Build detailed results by category
    nlohmann::json detailed_results = nlohmann::json::array();
    
    for (const auto& category : categories) {
      std::string category_name = category["category"].get<std::string>();
      int category_count = std::stoi(category["count"].get<std::string>());

      // Get problems in this category for this student
      nlohmann::json problems_in_category = nlohmann::json::array();
      for (const auto& problem : all_problems) {
        std::string prob_category = problem["soal_masalah_kategori"].get<std::string>();
        if (prob_category == category_name) {
          int prob_id = std::stoi(problem["soal_masalah_id"].get<std::string>());
          std::string prob_name = problem_names[prob_id];
          
          problems_in_category.push_back({
              {"problem_id", prob_id},
              {"problem_name", prob_name}
          });
        }
      }

      detailed_results.push_back({
          {"category", category_name},
          {"count", category_count},
          {"problems", problems_in_category}
      });
    }

    nlohmann::json response = {
        {"student_info", {
            {"nama", student_check[0]["nama"].get<std::string>()},
            {"kelas", kelas_nama},
            {"jurusan", jurusan}
        }},
        {"detailed_results", detailed_results}
    };

    DB_CLOSE();
    return Server.Response(connection, 200, "OK", response.dump());

  } catch (std::exception &e) {
    DB_CLOSE();
    return Server.Response(connection, 500, "Internal Server Error",
                           std::string(R"({"error":")") + e.what() + "\"}");
  }
}// ========================= UI: /dashboard/bk =========================
route("/dashboard/bk", db_bk) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::BK);

  if (!authInfo)
    return Server.ResponseAsFile(connection, 401, "Unauthorized",
                                 "public/401.html");

  Server.static_serve("public/views/bk/dashboard.html", connection);
  return 200;
}
