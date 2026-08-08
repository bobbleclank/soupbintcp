FetchContent_MakeAvailable(googletest)

add_executable(test_bcsouputility)
target_sources(test_bcsouputility
  PRIVATE
    option_convert_test.cpp
    option_error_test.cpp
)
target_link_libraries(test_bcsouputility
  PRIVATE
    bcsoup_warnings
    bcsouputility
    GTest::gtest_main
    GTest::gtest
)
target_compile_features(test_bcsouputility
  PRIVATE
    cxx_std_23
)

add_test(
  NAME test_bcsouputility
  COMMAND test_bcsouputility
)
