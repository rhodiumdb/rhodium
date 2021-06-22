cmake_minimum_required(VERSION 3.5)
project(rdss)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -g -O1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g -O1")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -g -O1")

add_library(rdss_logging
  src/logging/logging.cpp
  src/logging/log_entry.cpp
  src/logging/log_flags.cpp
  src/logging/log_message.cpp
  src/logging/strerror.cpp
  src/logging/symbolized_stacktrace.cpp
  src/logging/vlog_is_on.cpp)
target_link_libraries(rdss_logging
  absl::flags
  absl::time)

find_package(absl REQUIRED)
add_executable(rdss
  src/rdss.cpp
  src/ast.cpp)
target_link_libraries(rdss
  rdss_logging
  absl::hash
  absl::strings
  absl::status
  absl::statusor
  z3)
install(TARGETS rdss DESTINATION bin)

enable_testing()
add_executable(
  fhd_tests
  test/fhd_tests.cpp
)
target_link_libraries(
  fhd_tests
  rdss_logging
  absl::hash
  absl::strings
  absl::status
  absl::statusor
  z3
  gtest
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(fhd_tests)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp)
add_custom_target(
    clangformat
    COMMAND clang-format -style=LLVM -i ${ALL_SOURCE_FILES}
)
