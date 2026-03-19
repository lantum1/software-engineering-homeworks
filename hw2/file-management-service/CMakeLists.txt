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

file(GLOB_RECURSE ALL_SOURCES src/*.cpp)

list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(file-management-service-lib STATIC ${ALL_SOURCES})

target_include_directories(file-management-service-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(file-management-service-lib PRIVATE
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Crypto
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)

target_compile_options(file-management-service-lib PRIVATE
    -Wall -Wextra -Wpedantic
)

target_compile_definitions(file-management-service-lib PRIVATE
    POCO_UNBUNDLED
)

add_executable(file-management-service src/main.cpp)

target_link_libraries(file-management-service PRIVATE
    file-management-service-lib
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
    
    add_executable(file-management-service-tests
        tests/FileManagementServiceTest.cpp
    )
    
    target_link_libraries(file-management-service-tests PRIVATE
        file-management-service-lib
        GTest::gtest_main
    )
    
    target_include_directories(file-management-service-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    gtest_discover_tests(file-management-service-tests)
endif()