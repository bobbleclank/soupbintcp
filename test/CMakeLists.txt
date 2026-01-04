find_package(GTest CONFIG REQUIRED)
message("Found GTest: ${GTest_CONFIG} (found version \"${GTest_VERSION}\")")

add_executable(test_${PROJECT_NAME})
target_sources(test_${PROJECT_NAME}
  PRIVATE
    constants_test.cpp
    error_test.cpp
    expected_test.cpp
    file_store_test.cpp
    logical_packets_test.cpp
    message_test.cpp
    packing_test.cpp
    rw_packets_test.cpp
    validate_test.cpp
)
target_link_libraries(test_${PROJECT_NAME}
  PRIVATE
    bcsouptcp
    GTest::gtest_main
    GTest::gtest
)
target_compile_features(test_${PROJECT_NAME}
  PRIVATE
    cxx_std_23
)
target_compile_options(test_${PROJECT_NAME}
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

add_test(
  NAME ${PROJECT_NAME}
  COMMAND test_${PROJECT_NAME}
)
