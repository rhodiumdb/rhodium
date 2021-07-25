cmake_minimum_required(VERSION 3.5)
project(rdss)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -g -O1")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -g -O1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g -O1")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -g -O1")

find_package(PkgConfig REQUIRED)
pkg_check_modules(TREE_SITTER REQUIRED IMPORTED_TARGET tree-sitter)

pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

include_directories("${JSONCPP_INCLUDE_DIRS}")

add_library(rdss_parser
  deps/tree-sitter-rhodium/src/parser.c
  src/parser.cpp)
target_link_libraries(rdss_parser
  absl::flags
  absl::time
  PkgConfig::TREE_SITTER)

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

add_library(rdss_filesystem
  src/filesystem/temp_directory.cpp)
target_link_libraries(rdss_filesystem
  absl::strings
  absl::statusor)

find_package(absl REQUIRED)
add_executable(rdss
  src/rdss.cpp
  src/subprocess.cpp
  src/ast.cpp
  src/lsp.cpp)
target_link_libraries(rdss
  rdss_filesystem
  rdss_logging
  rdss_parser
  absl::hash
  absl::strings
  absl::status
  absl::statusor
  PkgConfig::JSONCPP
  z3
)
install(TARGETS rdss DESTINATION bin)

enable_testing()
add_executable(
  fhd_tests
  test/fhd_tests.cpp
)
target_link_libraries(
  fhd_tests
  rdss_filesystem
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
