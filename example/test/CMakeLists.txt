add_executable(test_utility
  option_convert_test.cpp
  option_error_test.cpp)
target_link_libraries(test_utility PUBLIC utility gtest_main gtest)
