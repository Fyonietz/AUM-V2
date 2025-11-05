
# ==============================================================
# Build "routes" as a shared library for runtime updates
# ==============================================================

file(GLOB_RECURSE PAGE_LIST "../pages/*.cpp")
file(GLOB_RECURSE LIBS "../libs/Phoenix/*.cpp")

add_library(routes SHARED
    ${PAGE_LIST}
    ${LIBS}
    register.cpp
)

# --------------------------------------------------------------
# Include directories
# --------------------------------------------------------------
target_include_directories(routes
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/libs
        ${CMAKE_SOURCE_DIR}/pages
        ${CMAKE_SOURCE_DIR}/core
        ${MYSQL_INCLUDE_DIR}   # MySQL headers
)

# --------------------------------------------------------------
# Link dependencies
# --------------------------------------------------------------
target_link_libraries(routes PRIVATE
    core_static
    civetweb
    pthread
    dl
    ssl
    crypto
    ${MYSQL_LIBRARY}          # Link to MySQL client library
)

# --------------------------------------------------------------
# Output configuration
# --------------------------------------------------------------
set_target_properties(routes PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/core"
    VERSION 1.0
    SOVERSION 1
)

# --------------------------------------------------------------
# Platform/compile definitions
# --------------------------------------------------------------
if (WIN32)
    add_definitions(-DNO_SSL -DWIN32_LEAN_AND_MEAN)
else()
    add_definitions(-DNO_SSL -DLINUX)
endif()

# --------------------------------------------------------------
# Post-build: copy shared library into /core
# --------------------------------------------------------------
add_custom_command(TARGET routes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:routes>
        ${CMAKE_SOURCE_DIR}/core/
    COMMENT "Copying routes library to core directory"
)

