add_executable(test_${PROJECT_NAME}
  constants_test.cpp
  error_test.cpp
  expected_test.cpp
  file_store_test.cpp
  logical_packets_test.cpp
  message_test.cpp
  packing_test.cpp
  rw_packets_test.cpp
  validate_test.cpp)
target_link_libraries(test_${PROJECT_NAME} PUBLIC bcsouptcp gtest_main gtest)
