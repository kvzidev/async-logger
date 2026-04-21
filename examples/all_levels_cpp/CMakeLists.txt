add_executable(all_levels_cpp main.cpp)
target_link_libraries(all_levels_cpp PRIVATE async_logger::async_logger)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(all_levels_cpp PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
