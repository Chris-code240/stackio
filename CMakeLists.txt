cmake_minimum_required(VERSION 3.15)
project(stackio)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp source/handler.cpp source/request.cpp source/router.cpp source/server.cpp source/storage_engine.cpp source/pugixml.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/includes"
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/source"
)


