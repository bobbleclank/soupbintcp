add_executable(test_bc_utility
  option_convert_test.cpp
  option_error_test.cpp)
target_link_libraries(test_bc_utility PUBLIC bcutility gtest_main gtest)
