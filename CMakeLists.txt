cmake_minimum_required(VERSION 3.25)

project(async_logger 
    VERSION 0.0.0 
    DESCRIPTION "C-Compatible Asynchronous Logging Library"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static analysis configuration
option(ASYNC_LOGGER_LINT "Enable clang-tidy linting during build" OFF)
if(ASYNC_LOGGER_LINT)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(NOT CLANG_TIDY_EXE)
        message(WARNING "clang-tidy enabled but executable not found")
    endif()
endif()

add_library(async_logger_warnings INTERFACE)

if(MSVC)
    target_compile_options(async_logger_warnings INTERFACE /W4 /WX /permissive-)
else()
    target_compile_options(async_logger_warnings INTERFACE 
        -Wall -Wextra -Wpedantic -Werror
        -Wshadow -Wnon-virtual-dtor -Wold-style-cast
    )
endif()

add_library(async_logger STATIC)
add_library(async_logger::async_logger ALIAS async_logger)

target_sources(async_logger
    PRIVATE
        src/logger.cpp
        src/logger_impl.cpp
        src/file_sink.cpp
        src/synchronized_queue.cpp
    PUBLIC FILE_SET HEADERS
        BASE_DIRS include
        FILES 
            include/logger.h
            include/logger
            include/async_logger/types.h
            include/async_logger/custom.h
)

target_include_directories(async_logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(async_logger
    PRIVATE
        $<BUILD_INTERFACE:async_logger_warnings>
)

find_package(Threads REQUIRED)
target_link_libraries(async_logger PUBLIC Threads::Threads)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(async_logger PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()


# Installation

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS async_logger
    EXPORT async_loggerTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT async_loggerTargets
    FILE async_loggerTargets.cmake
    NAMESPACE async_logger::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/async_loggerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/async_loggerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/async_logger
)

# Examples/Tests (Standalone projects)
# Only build if this is the top-level project
if(PROJECT_IS_TOP_LEVEL)
    # Testing infrastructure
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # For Windows: prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(examples/basic_c_usage)
    add_subdirectory(examples/basic_cpp_usage)
    add_subdirectory(examples/concurrent_usage)
    add_subdirectory(examples/all_levels_cpp)
    add_subdirectory(examples/custom_formatter)
    add_subdirectory(examples/memory_sink)
    add_subdirectory(tests)
endif()
