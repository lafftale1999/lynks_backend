cmake_minimum_required(VERSION 3.23)

project(lynks_backend LANGUAGES CXX)

add_executable(${PROJECT_NAME} main/main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    _WIN32_WINNT=0x0A00
    WINVERS=0x0A00
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    "C:/dev/boost"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ws2_32
    mswsock
)