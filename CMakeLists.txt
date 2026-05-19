# ── 1) Base project setup ─────────────────────────────────────────────────────
# Minimum CMake version required by cmake-js
cmake_minimum_required(VERSION 3.15...3.31)

# Project name (output binary becomes hello.node)
project(hello)

# ── 2) Node-API / addon behavior flags ───────────────────────────────────────
# NAPI_VERSION selects the Node-API surface this addon targets.
# Using v4 is a solid baseline (AsyncWorker support, stable since Node 10).
# ABI stability promise: build once against v4, run on runtimes supporting
# v4+ (Node.js/Bun) without recompiling.
add_compile_definitions(NAPI_VERSION=4)

# Enable C++ exceptions in node-addon-api mode.
# Without this, throwing C++ exceptions in addon code is undefined behavior.
# With this, exceptions are caught at the NAPI boundary and mapped to JS errors.
add_compile_definitions(NODE_ADDON_API_CPP_EXCEPTIONS)

# ── 3) Source collection ──────────────────────────────────────────────────────
# Collect all addon source files.
file(GLOB SOURCE_FILES "addon/*.cpp")

# ── 4) Build target (.node addon) ────────────────────────────────────────────
# SHARED + .node suffix = loadable Node/Bun addon.
# CMAKE_JS_SRC injects cmake-js required sources (e.g., win_delay_load_hook).
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Enforce C++17 (node-addon-api needs C++11+, C++17 is a modern sweet spot).
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# ── 5) Include/link paths from cmake-js ──────────────────────────────────────
# CMAKE_JS_INC provides the path to Node.js headers (napi.h lives here).
# CMAKE_JS_LIB provides the runtime link library.
# cmake-js computes both automatically based on your installed Node version.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})

# ── 6) Include path for node-addon-api ───────────────────────────────────────
# Locate and include node-addon-api headers.
# node-addon-api is a header-only C++ wrapper over the C Node-API.
# We ask Node to print the include path, then strip quotes and newlines.
execute_process(
    COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
)
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

# ── 7) Windows-only node.lib generation (MSVC) ───────────────────────────────
# Windows-only: generate node.lib from the .def file that cmake-js provides.
# This is required for the addon to link correctly on MSVC. No-op on macOS/Linux.
if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    execute_process(COMMAND ${CMAKE_AR}
        /def:${CMAKE_JS_NODELIB_DEF}
        /out:${CMAKE_JS_NODELIB_TARGET}
        ${CMAKE_STATIC_LINKER_FLAGS}
    )
endif()