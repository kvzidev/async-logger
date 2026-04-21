add_executable(basic_cpp_usage main.cpp)
target_link_libraries(basic_cpp_usage PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(basic_cpp_usage PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
