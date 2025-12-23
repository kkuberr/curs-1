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
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(Qt6_FOUND)
    set(HAVE_QT TRUE)
else()
    find_package(Qt5 COMPONENTS Core Widgets QUIET)
    if(Qt5_FOUND)
        set(HAVE_QT TRUE)
    else()
        set(HAVE_QT FALSE)
    endif()
endif()

# ===== open62541 =====
find_package(open62541 CONFIG QUIET)
if(open62541_FOUND)
    add_compile_definitions(WITH_OPEN62541)
endif()

# ===== Paths =====
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(UA_DIR ${SRC_DIR}/ua)

# ===== Library =====
add_library(opcua_client STATIC
    ${SRC_DIR}/OpcUaClient.cpp
    ${UA_DIR}/MockUaClient.cpp
    ${UA_DIR}/Open62541Client.cpp
)

target_include_directories(opcua_client 
    PUBLIC 
        ${SRC_DIR} 
        ${UA_DIR}
)

if(open62541_FOUND)
    target_link_libraries(opcua_client PUBLIC open62541::open62541)
    target_compile_definitions(opcua_client PUBLIC WITH_OPEN62541)
    # Explicitly add include directories for open62541 headers
    get_target_property(_open62541_include_dirs open62541::open62541 INTERFACE_INCLUDE_DIRECTORIES)
    if(_open62541_include_dirs)
        target_include_directories(opcua_client PUBLIC ${_open62541_include_dirs})
        message(STATUS "Added open62541 include dirs: ${_open62541_include_dirs}")
    else()
        # Fallback: add common vcpkg path
        target_include_directories(opcua_client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/include")
    endif()
else()
    message(WARNING "open62541 not found. Building without OPC UA support.")
endif()

# Fallback: if open62541 CMake config wasn't found but vcpkg is present, add vcpkg include path
if(NOT open62541_FOUND AND DEFINED ENV{VCPKG_ROOT})
    set(_vcpkg_root $ENV{VCPKG_ROOT})
    # try common triplet folder
    set(_vcpkg_inc "${_vcpkg_root}/installed/x64-windows/include")
    if(EXISTS "${_vcpkg_inc}")
        message(STATUS "Adding vcpkg include path for open62541: ${_vcpkg_inc}")
        target_include_directories(opcua_client PUBLIC "${_vcpkg_inc}")
    endif()
endif()

# ===== Executable =====
if(HAVE_QT)
    add_executable(opcua_qt_client
        ${SRC_DIR}/main.cpp
        ${SRC_DIR}/mainwindow.cpp
    )

    if(Qt6_FOUND)
        target_link_libraries(opcua_qt_client
            PRIVATE
                opcua_client
                Qt6::Widgets
        )
    else()
        target_link_libraries(opcua_qt_client
            PRIVATE
                opcua_client
                Qt5::Widgets
        )
    endif()
else()
    message(WARNING "Qt not found. GUI executable will not be built.")
endif()

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
