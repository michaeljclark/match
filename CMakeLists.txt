cmake_minimum_required(VERSION 3.0.2)

project(match)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)

macro(add_compiler_flags)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
endmacro(add_compiler_flags)

check_cxx_compiler_flag("-pg" has_gprof "int main() { return 0; }")
if (CMAKE_PROFILE AND has_gprof)
  add_compiler_flags(-pg)
endif()

add_executable(match src/match.cc)
