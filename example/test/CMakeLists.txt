add_executable(test_bc_utility)
target_sources(test_bc_utility
  PRIVATE
    option_convert_test.cpp
    option_error_test.cpp
)
target_link_libraries(test_bc_utility PRIVATE bcutility gtest_main gtest)
