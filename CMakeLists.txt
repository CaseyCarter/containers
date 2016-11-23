# cmcstl2 - A concept-enabled C++ standard library
#
#  Copyright Casey Carter 2015
#
#  Use, modification and distribution is subject to the
#  Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/caseycarter/cmcstl2
#
cmake_minimum_required(VERSION 3.0)

project(containers CXX)

enable_testing()
include(CTest)

include_directories(include)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmcstl2/include)
  find_package(Git)
  if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update --init WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  else()
    message(STATUS "OH NO!")
  endif()
endif()

include_directories(cmcstl2/include)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fconcepts -ftemplate-backtrace-limit=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -march=native -mtune=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline -g3 -fstack-protector-all")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG")
endif()

add_subdirectory(test)
