
cmake_minimum_required(VERSION 3.10)
project(AUM-Phoenix)

# ----------------------
# Force generic CPU instructions
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure CPU compatibility for older machines
if(NOT CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=x86-64 -mtune=generic")

# ----------------------
# Source files
# ----------------------
file(GLOB_RECURSE MAIN "main.cpp")
file(GLOB_RECURSE CIVET_C "libs/Civet/civetweb.c")
file(GLOB_RECURSE CIVET_INL "libs/Civet/*.inl")

file(GLOB_RECURSE CORE_SOURCES
    "core/*.cpp"
    "models/*.cpp"
    "libs/Phoenix/mysql.cpp"  # MySQL implementation (optional)
)

# ----------------------
# Find dependencies
# ----------------------

# MySQL
find_path(MYSQL_INCLUDE_DIR mysql.h
    PATHS /usr/include/mysql /usr/local/include/mysql /opt/homebrew/include/mysql /usr/include
)
find_library(MYSQL_LIBRARY mysqlclient
    PATHS /usr/lib/mysql /usr/local/lib/mysql /opt/homebrew/lib/mysql /usr/lib/x86_64-linux-gnu
)

# SQLite3
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
find_library(SQLITE3_LIBRARY sqlite3)

# Determine which database to use
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "Found MySQL include: ${MYSQL_INCLUDE_DIR}")
    message(STATUS "Found MySQL library: ${MYSQL_LIBRARY}")
    add_definitions(-DUSE_MYSQL)
elseif(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
    message(STATUS "Using SQLite3: ${SQLITE3_LIBRARY}")
    add_definitions(-DUSE_SQLITE)
else()
    message(FATAL_ERROR "Neither MySQL nor SQLite3 found!")
endif()

# ----------------------
# CivetWeb static library
# ----------------------
add_library(civetweb STATIC ${CIVET_C} ${CIVET_INL})
target_include_directories(civetweb PUBLIC libs)
add_definitions(-DNO_SSL)

# ----------------------
# Core static library
# ----------------------
add_library(core_static STATIC ${CORE_SOURCES})
set_target_properties(core_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Core includes
target_include_directories(core_static
    PRIVATE core
    PRIVATE libs
    PUBLIC /usr/include
)

if(MYSQL_INCLUDE_DIR)
    target_include_directories(core_static PUBLIC ${MYSQL_INCLUDE_DIR})
endif()
if(SQLITE3_INCLUDE_DIR)
    target_include_directories(core_static PUBLIC ${SQLITE3_INCLUDE_DIR})
endif()

target_link_libraries(core_static PRIVATE civetweb)

# ----------------------
# Routes subdirectory
# ----------------------
add_subdirectory(routes)

# ----------------------
# Main executable
# ----------------------
add_executable(${PROJECT_NAME} ${MAIN})

target_include_directories(${PROJECT_NAME}
    PRIVATE core
    PRIVATE libs
    PRIVATE models/
    PUBLIC /usr/include/
)

if(MYSQL_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PUBLIC ${MYSQL_INCLUDE_DIR})
endif()
if(SQLITE3_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PUBLIC ${SQLITE3_INCLUDE_DIR})
endif()

# Link main executable
target_link_libraries(${PROJECT_NAME} PRIVATE core_static pthread dl)

if(SQLITE3_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLITE3_LIBRARY})
endif()
if(MYSQL_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQL_LIBRARY})
endif()

# ----------------------
# Platform-specific definitions
# ----------------------
if(WIN32)
    add_definitions(-DNO_SSL)
else()
    add_definitions(-DNO_SSL -DLINUX)
endif()



