enable_testing()

add_executable(async_logger_tests
    test_queue.cpp
    test_logger.cpp
)

target_link_libraries(async_logger_tests
    PRIVATE
        async_logger::async_logger
        GTest::gtest_main
)

if(ASYNC_LOGGER_LINT AND CLANG_TIDY_EXE)
    set_target_properties(async_logger_tests PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

include(GoogleTest)
gtest_discover_tests(async_logger_tests)
