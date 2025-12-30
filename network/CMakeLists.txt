cmake_minimum_required(VERSION 3.21)
project(lynks_backend LANGUAGES CXX)

include(FetchContent)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/main/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main/repo/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main/service/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main/janus/src/*.cpp"
)

add_executable(${PROJECT_NAME}
    main/main.cpp
    ${APP_SOURCES}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# ---- Threads + OpenSSL ----
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# ---- nlohmann/json (header-only) via FetchContent ----
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    ${CMAKE_CURRENT_SOURCE_DIR}/main/repo
    ${CMAKE_CURRENT_SOURCE_DIR}/main/service
    ${CMAKE_CURRENT_SOURCE_DIR}/main/janus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/secret
)

# ---- Build Boost.Charconv from Boost source tree ----
# Expect BOOST_ROOT to be provided (e.g. from Dockerfile)
if(NOT DEFINED BOOST_ROOT)
  set(BOOST_ROOT "" CACHE PATH "Path to Boost source root (containing boost/ and libs/)")
endif()

if(BOOST_ROOT STREQUAL "")
  message(FATAL_ERROR "BOOST_ROOT is not set. Provide -DBOOST_ROOT=/path/to/boost_1_xx_0")
endif()

add_library(boost_charconv STATIC
    ${BOOST_ROOT}/libs/charconv/src/from_chars.cpp
    ${BOOST_ROOT}/libs/charconv/src/to_chars.cpp
)

target_include_directories(boost_charconv PUBLIC
    ${BOOST_ROOT}
)

target_compile_definitions(boost_charconv PUBLIC BOOST_CHARCONV_NO_LIB)

target_link_libraries(${PROJECT_NAME} PRIVATE
    boost_charconv
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# Nice-to-have: stricter warnings on GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()