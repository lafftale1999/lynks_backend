cmake_minimum_required(VERSION 3.21)
project(lynks_backend LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    main/main.cpp
    main/src/network_crypto.cpp
    main/src/network_user.cpp
    main/src/network_mysql.cpp
    main/repo/user_repo.cpp
    main/service/user_service.cpp
    main/src/network_session_handler.cpp
    main/src/network_session_token.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _WIN32_WINNT=0x0A00
    WINVERS=0x0A00
)

# ---- OpenSSL + Threads ----
set(OPENSSL_ROOT_DIR "C:/dev/msys64/mingw64")
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    ${CMAKE_CURRENT_SOURCE_DIR}/main/repo
    ${CMAKE_CURRENT_SOURCE_DIR}/main/service
    ${CMAKE_CURRENT_SOURCE_DIR}/secret

    # Boost headers
    C:/dev/boost
    C:/dev/cpp_external_libs/nlohmann/include
)

# ---- Build Boost.Charconv from source ----
add_library(boost_charconv STATIC
    C:/dev/boost/libs/charconv/src/from_chars.cpp
    C:/dev/boost/libs/charconv/src/to_chars.cpp
)

target_include_directories(boost_charconv PUBLIC
    C:/dev/boost
)

# Ofta bra att slå av auto-linking-beteende för Boost libs på Windows:
target_compile_definitions(boost_charconv PUBLIC BOOST_CHARCONV_NO_LIB)

target_link_libraries(${PROJECT_NAME} PRIVATE
    boost_charconv
    ws2_32
    mswsock
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
