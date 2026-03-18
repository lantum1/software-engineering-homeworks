cmake_minimum_required(VERSION 3.16)

project(file-management-service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Net Util JSON Crypto)
find_package(OpenSSL REQUIRED)

include(FetchContent)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(file-management-service ${SOURCES})

target_include_directories(file-management-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(file-management-service PRIVATE
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Crypto
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)

target_compile_options(file-management-service PRIVATE
    -Wall -Wextra -Wpedantic
)

target_compile_definitions(file-management-service PRIVATE
    POCO_UNBUNDLED
)