FetchContent_MakeAvailable(googletest)

add_executable(test_bcsoup)
target_sources(test_bcsoup
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
target_link_libraries(test_bcsoup
  PRIVATE
    bcsoup
    GTest::gtest_main
    GTest::gtest
)
target_compile_features(test_bcsoup
  PRIVATE
    cxx_std_23
)
target_compile_options(test_bcsoup
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

add_test(
  NAME test_bcsoup
  COMMAND test_bcsoup
)
