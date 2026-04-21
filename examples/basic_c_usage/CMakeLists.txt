add_executable(basic_c_usage main.c)
target_link_libraries(basic_c_usage PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(basic_c_usage PROPERTIES C_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
