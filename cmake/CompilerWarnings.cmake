add_library(bcsoup_warnings INTERFACE)
target_compile_options(bcsoup_warnings
  INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    $<BUILD_INTERFACE:-Werror>
)
