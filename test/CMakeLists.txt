add_executable(test_soup
  packing_test.cpp)
target_link_libraries(test_soup PUBLIC soup gtest_main gtest)
