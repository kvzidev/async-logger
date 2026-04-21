add_executable(memory_sink main.cpp)
target_link_libraries(memory_sink PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(memory_sink PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
