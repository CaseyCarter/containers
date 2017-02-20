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
#add_executable(headers headers.cpp headers2.cpp)
#add_test(test.headers headers)

#add_executable(forward_list forward_list.cpp)
#add_test(test.forward_list forward_list)

#add_executable(vector vector.cpp)
#add_test(test.vector vector)

add_executable(allocator allocator.cpp)
target_compile_definitions(allocator PRIVATE -D_ISOC11_SOURCE)
add_test(test.allocator allocator)

add_subdirectory(container)
