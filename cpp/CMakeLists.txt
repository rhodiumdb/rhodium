cmake_minimum_required(VERSION 3.5)
project(rdss)
set(CMAKE_CXX_STANDARD 17)
find_package(absl REQUIRED)
add_executable(rdss
  src/rdss.cpp
  src/ast.cpp)
target_link_libraries(rdss absl::strings absl::status absl::statusor)
