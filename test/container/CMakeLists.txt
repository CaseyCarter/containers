# cmcstl2 - A concept-enabled C++ standard library
#
#  Copyright Casey Carter 2015
#  Copyright Christopher Di Bella 2017
#
#  Use, modification and distribution is subject to the
#  Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/caseycarter/cmcstl2
#

add_executable(container container.cpp
                         container2.cpp
                         container3.cpp
                         reversible_container.cpp
                         allocator_aware.cpp)
target_compile_definitions(container PRIVATE -DVALIDATE_STL2)
add_test(test.container container)
