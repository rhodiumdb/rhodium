cmake_minimum_required(VERSION 3.5)
project(rdss)
set(CMAKE_CXX_STANDARD 17)
find_package(absl REQUIRED)
add_executable(rdss
  src/rdss.cpp
  src/ast.cpp)
target_link_libraries(rdss
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
  absl::hash
  absl::strings
  gtest
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(fhd_tests)
