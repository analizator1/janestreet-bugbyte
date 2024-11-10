cmake_minimum_required(VERSION 3.14)

project(bugbyte)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wvla -Winit-self -Wnon-virtual-dtor -Woverloaded-virtual)

add_executable(bugbyte
	main.cpp
	permutations.cpp
	utils.cpp
)

add_executable(heap_test
	heap_test.cpp
)

add_executable(permutations_test
	permutations_test.cpp
	permutations.cpp
)
