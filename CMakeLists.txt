cmake_minimum_required(VERSION 3.16)
project(opcua_qt_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Автоматические возможности Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ===== ВАРИАНТ А: Жесткий путь (работает у всех на Windows) =====
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# ===== ВАРИАНТ Б: Умный поиск (лучше!) =====
if(WIN32 AND EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    # Для всех Windows пользователей
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using system vcpkg: C:/vcpkg")
elseif(DEFINED ENV{VCPKG_ROOT})
    # Для пользователей с переменной окружения
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
else()
    # Ищем в других местах
    message(WARNING "vcpkg not found. Using system packages...")
endif()

# Поиск Qt6, затем fallback на Qt5 (позволяет использовать qt5 из vcpkg)
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(Qt6_FOUND)
    set(HAVE_QT TRUE)
    set(QT_WIDGETS_TARGET Qt6::Widgets)
    message(STATUS "Found Qt6: building GUI target")
else()
    find_package(Qt5 COMPONENTS Core Widgets QUIET)
    if(Qt5_FOUND)
        set(HAVE_QT TRUE)
        set(QT_WIDGETS_TARGET Qt5::Widgets)
        message(STATUS "Found Qt5: building GUI target")
    else()
        set(HAVE_QT FALSE)
        message(WARNING "Qt6/Qt5 not found. GUI target will be skipped. Tests will still build.")
    endif()
endif()
option(BUILD_TESTS "Build unit tests" ON)

# project sources
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(OPCUA_CLIENT_SOURCES
    ${SRC_DIR}/opcua_client.cpp
)

add_library(opcua_client STATIC ${OPCUA_CLIENT_SOURCES})
target_include_directories(opcua_client PUBLIC ${SRC_DIR})
target_compile_features(opcua_client PUBLIC cxx_std_17)

# main application
set(MAIN_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/mainwindow.cpp
)

if(HAVE_QT)
    add_executable(opcua_qt_client ${MAIN_SOURCES})
    target_link_libraries(opcua_qt_client PRIVATE opcua_client ${QT_WIDGETS_TARGET})
endif()

# Tests (optional)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    # For Windows: prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_opcua_client test/test_opcua_client.cpp)
    target_link_libraries(test_opcua_client PRIVATE opcua_client gtest_main)
    target_include_directories(test_opcua_client PRIVATE ${SRC_DIR})

    include(GoogleTest)
    gtest_discover_tests(test_opcua_client)
endif()