cmake_minimum_required(VERSION 3.16)

# ===== vcpkg toolchain =====
if(WIN32 AND EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain")
elseif(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain")
endif()

project(opcua_qt_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ===== Qt =====
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ===== open62541 =====
find_package(open62541 CONFIG REQUIRED)
add_compile_definitions(WITH_OPEN62541)

# ===== Paths =====
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(UA_DIR ${SRC_DIR}/ua)

# ===== Library =====
add_library(opcua_client STATIC
    ${SRC_DIR}/OpcUaClient.cpp
    ${UA_DIR}/MockUaClient.cpp
    ${UA_DIR}/Open62541Client.cpp
)

target_include_directories(opcua_client PUBLIC ${SRC_DIR} ${UA_DIR})
target_link_libraries(opcua_client PUBLIC open62541::open62541)

# ===== Executable =====
add_executable(opcua_qt_client
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/mainwindow.cpp
)

target_link_libraries(opcua_qt_client
    PRIVATE
        opcua_client
        Qt6::Widgets
)

# ===== Tests =====
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(test_opcua_client test/test_opcua_client.cpp)
    target_link_libraries(test_opcua_client PRIVATE opcua_client gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_opcua_client)
endif()
