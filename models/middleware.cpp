#include "middleware.hpp"
#include <sstream>
#include <unordered_map>

namespace Middleware {

std::string Auth::roleToStr(Roles roles) {
  static const std::unordered_map<Roles, std::string> map = {
      {Roles::Admin, "admin"},
      {Roles::BK, "BK"},
      {Roles::Siswa, "siswa"},
      {Roles::None, "none"},
      {Roles::Unknown, "unknown"}};

  auto it = map.find(roles);
  return (it != map.end()) ? it->second : "unknown";
};

Auth::Roles Auth::strToRole(const std::string &user) {
  static const std::unordered_map<std::string, Roles> map = {
      {"Admin", Roles::Admin}, {"BK", Roles::BK}, {"Siswa", Roles::Siswa}};

  auto it = map.find(user);
  return (it != map.end()) ? it->second : Roles::Unknown;
}

std::vector<Auth::Roles> Auth::bitmaskToRoles(Roles roleMask) {
  std::vector<Roles> roles;
  auto maskValue = static_cast<std::underlying_type_t<Roles>>(roleMask);

  if (maskValue & static_cast<std::underlying_type_t<Roles>>(Roles::Admin))
    roles.push_back(Roles::Admin);
  if (maskValue & static_cast<std::underlying_type_t<Roles>>(Roles::Siswa))
    roles.push_back(Roles::Siswa);
  if (maskValue & static_cast<std::underlying_type_t<Roles>>(Roles::BK))
    roles.push_back(Roles::BK);

  return roles;
}

bool Auth::hasRole(Roles userRole, Roles requiredRoleMask) {
  if (requiredRoleMask == Roles::None) {
    return true; // No specific role required
  }

  return (userRole & requiredRoleMask) != Roles::None;
}

std::string Auth::tokenizer(const std::string &input) {
  // Create a context for the EVP hashing
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  const EVP_MD *md = EVP_sha256(); // Specify SHA256

  // Initialize the context
  if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("Failed to initialize digest context.");
  }

  // Update the context with input data
  if (EVP_DigestUpdate(ctx, input.c_str(), input.size()) != 1) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("Failed to update digest.");
  }

  // Prepare to store the hash result
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len;

  // Finalize the digest (get the result)
  if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("Failed to finalize digest.");
  }

  // Free the context
  EVP_MD_CTX_free(ctx);

  // Convert hash to a hexadecimal string
  std::stringstream ss;
  for (unsigned int i = 0; i < hash_len; i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)hash[i];
  }

  return ss.str();
}

} // namespace Middleware
