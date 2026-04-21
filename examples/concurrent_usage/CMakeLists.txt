add_executable(concurrent_usage main.cpp)
target_link_libraries(concurrent_usage PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(concurrent_usage PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
