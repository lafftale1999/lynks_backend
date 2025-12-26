cmake_minimum_required(VERSION 3.5)

project(lynks_backend LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS charconv)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} 
    main/main.cpp 
    main/src/network_crypto.cpp
    main/src/network_user.cpp    
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    _WIN32_WINNT=0x0A00
    WINVERS=0x0A00
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    "C:/dev/boost"
    "C:/dev/cpp_external_libs/nlohmann/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ws2_32
    mswsock
    Boost::charconv
    Threads::Threads
    OpenSSL::Crypto
    OpenSSL::SSL
)