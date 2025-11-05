#include <string>

#include <string>

struct BidangMasalah {
  std::string id;
  std::string nama_bidang_masalah;
};

struct SoalMasalah {
  std::string id;
  std::string nama_bidang_masalah;
  std::string nama_soal_masalah;
};

struct Role {
  std::string id;
  std::string nama;
};

struct Kelas {
  std::string id;
  std::string nama;
  std::string jurusan; // optional
};

struct User {
  std::string id;
  std::string nama;
  std::string role_id;
  std::string password;
  std::string token;
  std::string kelas_id;
  std::string jurusan; // optional
};

struct Hasil {
  std::string id;
  std::string user_id;
  std::string soal_masalah_id;
  std::string soal_masalah_kategori;
};
