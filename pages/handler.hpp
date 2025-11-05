#pragma once

#include "../core/database.hpp"
#include "../core/engine.hpp"
#include "../core/starter.hpp"
#include "../libs/Phoenix/controller.hpp"
#include "../libs/Phoenix/mysql.hpp"
#include "../models/middleware.hpp"
#include "../models/project_model.hpp"
#include "../routes/register.hpp"
#include "pyro.hpp"
#include <string>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern Pnix Server;
extern Global test;
extern const std::string DATABASE;

template <typename... Args> void print(Args &&...args) {
  (std::cout << ... << args) << std::endl;
}

// Forward declarations
struct mg_connection;

EXPORT int default_handler(struct mg_connection *connection, void *);
EXPORT int home(struct mg_connection *connection, void *callback);

// CRITICAL CHANGE: CheckAuthToken now requires connection to be already open
// and does NOT close it
EXPORT std::optional<nlohmann::json> CheckAuthToken(
    struct mg_connection *conn,
    Middleware::Auth::Roles requiredRoles = Middleware::Auth::Roles::None);

EXPORT std::string GetAuthToken(struct mg_connection *connection);
EXPORT int strToIdRole(const std::string &role);

#define route(PATH, NAME)                                                      \
  int NAME(struct mg_connection *connection, void *cb);                        \
  namespace {                                                                  \
  struct NAME##_Reg {                                                          \
    NAME##_Reg() { add_route(PATH, NAME); }                                    \
  } NAME##_instance;                                                           \
  }                                                                            \
  int NAME(struct mg_connection *connection, void *cb)
