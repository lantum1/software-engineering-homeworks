cmake_minimum_required(VERSION 3.16)

project(file-management-service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Net Util JSON Crypto)
find_package(OpenSSL REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include(FetchContent)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE ALL_SOURCES src/*.cpp)

list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(file-management-service-lib STATIC ${ALL_SOURCES})

target_include_directories(file-management-service-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(file-management-service-lib PRIVATE
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Crypto
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
    ${HIREDIS_LIBRARIES}
)

target_link_libraries(file-management-service-lib PUBLIC
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
)

target_compile_options(file-management-service-lib PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(file-management-service src/main.cpp)

target_link_libraries(file-management-service PRIVATE
    file-management-service-lib
)