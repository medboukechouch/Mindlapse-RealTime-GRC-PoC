cmake_minimum_required(VERSION 3.14)
project(cyber_sentinel_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(log_parser log_parser.cpp)

if(MSVC)
  target_compile_options(log_parser PRIVATE /O2)
else()
  target_compile_options(log_parser PRIVATE -O3)
endif()
