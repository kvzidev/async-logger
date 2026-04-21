add_executable(custom_formatter main.cpp)
target_link_libraries(custom_formatter PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(custom_formatter PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
