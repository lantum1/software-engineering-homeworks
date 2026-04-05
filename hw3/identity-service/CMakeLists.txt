cmake_minimum_required(VERSION 3.16)

project(identity-service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Net Util JSON Data DataPostgreSQL)
find_package(OpenSSL REQUIRED)

find_library(PQ_LIBRARY NAMES pq REQUIRED)
find_path(PQ_INCLUDE_DIR NAMES libpq-fe.h PATHS /usr/include/postgresql REQUIRED)

if(NOT TARGET PostgreSQL::client)
    add_library(PostgreSQL::client UNKNOWN IMPORTED)
    set_target_properties(PostgreSQL::client PROPERTIES
        IMPORTED_LOCATION "${PQ_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PQ_INCLUDE_DIR}"
    )
endif()

include(FetchContent)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE ALL_SOURCES src/*.cpp)

list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(identity-service-lib STATIC ${ALL_SOURCES})

target_include_directories(identity-service-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(identity-service-lib PRIVATE
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Data
    Poco::DataPostgreSQL
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)

target_compile_options(identity-service-lib PRIVATE
    -Wall -Wextra -Wpedantic
)

target_compile_definitions(identity-service-lib PRIVATE
    POCO_UNBUNDLED
)

add_executable(identity-service src/main.cpp)

target_link_libraries(identity-service PRIVATE
    identity-service-lib
)

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    include(GoogleTest)
    
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
    
    add_executable(identity-service-tests ${TEST_SOURCES})
    
    target_link_libraries(identity-service-tests PRIVATE
        identity-service-lib
        GTest::gtest_main
    )
    
    target_include_directories(identity-service-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    
    if(TARGET PostgreSQL::client)
        target_link_libraries(identity-service-tests PRIVATE PostgreSQL::client)
    endif()
    
    target_compile_definitions(identity-service-tests PRIVATE
        POCO_UNBUNDLED
    )
    
    gtest_discover_tests(identity-service-tests)
endif()