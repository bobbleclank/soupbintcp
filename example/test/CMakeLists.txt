FetchContent_MakeAvailable(googletest)

add_executable(test_bc_utility)
target_sources(test_bc_utility
  PRIVATE
    option_convert_test.cpp
    option_error_test.cpp
)
target_link_libraries(test_bc_utility
  PRIVATE
    bcutility
    GTest::gtest_main
    GTest::gtest
)
target_compile_features(test_bc_utility
  PRIVATE
    cxx_std_23
)
target_compile_options(test_bc_utility
  PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

include(GoogleTest)
gtest_discover_tests(test_bc_utility)
