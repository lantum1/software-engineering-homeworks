cmake_minimum_required(VERSION 3.16)

project(identity-service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Net Util JSON)

find_package(OpenSSL REQUIRED)

include(FetchContent)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(identity-service ${SOURCES})

target_include_directories(identity-service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(identity-service PRIVATE
    Poco::Net
    Poco::Util
    Poco::JSON
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)

target_compile_options(identity-service PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(identity-service PRIVATE
    POCO_UNBUNDLED
)