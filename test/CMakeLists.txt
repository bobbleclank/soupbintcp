add_executable(test_soup
  constants_test.cpp
  packing_test.cpp
  rw_packets_test.cpp)
target_link_libraries(test_soup PUBLIC soup gtest_main gtest)
