
#include "starter.hpp"
#include "../libs/Phoenix/mysql.hpp" // Add this include
#include "../pages/pyro.hpp"

Pnix server;
std::string Global::dll_name = "core/libroutes.so";
std::string Global::info = " > Phoenix[Info]: ";
std::unordered_map<std::string, std::string> loadConfig() {
  std::unordered_map<std::string, std::string> config;
  std::ifstream file("config/server.wpc");

  if (!file) {
    std::cerr << "Failed To Open Config File: config/server.wpc" << std::endl;
    return config;
  }

  std::cout << "Config file opened successfully" << std::endl;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream input_string(line);
    std::string key, value;
    if (std::getline(input_string, key, '=') &&
        std::getline(input_string, value)) {
      config[key] = value;
      std::cout << "Loaded config: " << key << " = " << value << std::endl;
    }
  }

  std::cout << "Total config entries loaded: " << config.size() << std::endl;
  return config;
} // Helper function to safely get values from unordered_map with defaults
std::string
getConfigValue(const std::unordered_map<std::string, std::string> &config,
               const std::string &key, const std::string &defaultValue = "") {
  auto it = config.find(key);
  return (it != config.end()) ? it->second : defaultValue;
}
void initConfig() {
  auto config = loadConfig();

  // Check if configuration was loaded successfully
  if (config.empty()) {
    std::cerr << Global::info << "Failed to load configuration\n";
    return;
  }

  // Server Configuration
  Config::root = getConfigValue(config, "Document_Root");
  Config::port = getConfigValue(config, "Server_Port");
  Config::threads = getConfigValue(config, "Number_Threads");
  Config::keep_alive = getConfigValue(config, "Keep_Alive");
  Config::routes = getConfigValue(config, "Routes_Type");

  // Database Configuration - FORCE MySQL only
  Config::db_type = "mysql";
  Config::mysql_host = getConfigValue(config, "MySQL_Host", "localhost");
  Config::mysql_user = getConfigValue(config, "MySQL_User", "root");
  Config::mysql_password = getConfigValue(config, "MySQL_Password", "");
  Config::mysql_database =
      getConfigValue(config, "MySQL_Database", "aum_phoenix");
  Config::mysql_port =
      getConfigValue(config, "MySQL_Port", "3306"); // Ensure default

  // Debug output
  std::cout << Global::info << "Loaded MySQL Config:" << std::endl;
  std::cout << "  Host: " << Config::mysql_host << std::endl;
  std::cout << "  User: " << Config::mysql_user << std::endl;
  std::cout << "  Port: '" << Config::mysql_port << "'" << std::endl;
  std::cout << "  Database: " << Config::mysql_database << std::endl;

  // Validate configuration
  if (Config::root.empty()) {
    std::cerr << Global::info << "Missing Document_Root in configuration\n";
  }
  if (Config::port.empty()) {
    std::cerr << Global::info << "Missing Server_Port in configuration\n";
  }
}
void updateConfig() {
  // Ensure all required configuration values are set
  if (Config::root.empty() || Config::port.empty() || Config::threads.empty() ||
      Config::keep_alive.empty()) {
    std::cerr << Global::info << "Missing required configuration values\n";
    return;
  }

  // Verify public directory exists and is accessible
  try {
    if (!std::filesystem::exists(Config::root)) {
      std::filesystem::create_directories(Config::root);
      std::cout << Global::info << "Created directory: " << Config::root
                << "\n";
    }

    // Check if directory is readable
    if (!std::filesystem::is_directory(Config::root) ||
        access(Config::root.c_str(), R_OK) != 0) {
      std::cerr << Global::info << "Directory not accessible: " << Config::root
                << "\n";
      return;
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << Global::info
              << "Failed to create/access directory: " << e.what() << "\n";
    return;
  }

  // Initialize callbacks structure
  memset(&Config::callbacks, 0, sizeof(Config::callbacks));

  // Set up server configuration with correct port format
  const char *options[] = {"document_root",
                           Config::root.c_str(),
                           "listening_ports",
                           Config::port.c_str(), // <-- use config value!
                           "num_threads",
                           Config::threads.c_str(),
                           "enable_keep_alive",
                           Config::keep_alive.c_str(),
                           "index_files",
                           "layout.html",
                           nullptr};

  // Print configuration for debugging
  std::cout << Global::info << "Starting server with configuration:\n";
  for (int i = 0; options[i] != nullptr; i += 2) {
    std::cout << "  " << options[i] << ": " << options[i + 1] << "\n";
  }

  // Start the CivetWeb server
  Config::context = mg_start(&Config::callbacks, nullptr, options);

  if (!Config::context) {
    std::cerr << Global::info << "Failed to start server: " << strerror(errno)
              << "\n";
    if (std::filesystem::exists("error.log")) {
      std::ifstream log("error.log");
      std::string line;
      while (std::getline(log, line)) {
        std::cerr << Global::info << "Error log: " << line << "\n";
      }
    }
  } else {
    std::cout << Global::info
              << "Server started successfully on http://localhost:"
              << Config::port << std::endl;
  }
}

const char *Config::server_config[] = {"document_root",
                                       nullptr,
                                       "listening_ports",
                                       nullptr,
                                       "num_threads",
                                       nullptr,
                                       "enable_keep_alive",
                                       nullptr,
                                       "enable_directory_listing",
                                       "no",
                                       "index_files",
                                       "layout.html",
                                       nullptr};
