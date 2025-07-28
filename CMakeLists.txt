cmake_minimum_required(VERSION 3.15)
project(TypelusPlus) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For cpp-terminal
add_subdirectory(cpp-terminal)

# For fmt
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)
FetchContent_MakeAvailable(fmt)
# For
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)
FetchContent_MakeAvailable(fmt)

add_executable("type++" src/main.cpp)

target_link_libraries("type++" PRIVATE fmt::fmt cpp-terminal)
