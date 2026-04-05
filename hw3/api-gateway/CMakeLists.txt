cmake_minimum_required(VERSION 3.16)
project(max-api-gateway)

set(CMAKE_CXX_STANDARD 20)

find_package(Poco REQUIRED Net Util JSON)

include(FetchContent)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git  
    GIT_TAG master
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(jwt-cpp nlohmann_json)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(max-api-gateway ${SOURCES})

target_link_libraries(max-api-gateway
    Poco::Net
    Poco::Util
    Poco::JSON
    jwt-cpp::jwt-cpp
    nlohmann_json::nlohmann_json
)