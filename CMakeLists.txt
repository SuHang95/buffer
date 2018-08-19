cmake_minimum_required(VERSION 3.5)
project(IOBuffer)

set(CMAKE_CXX_STANDARD 11)

include_directories(".." ".")
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-O3)
    add_compile_options(-D _DEBUG)
endif(CMAKE_COMPILER_IS_GNUCXX)
find_package(Threads)
add_executable(IOBuffer
        buffer.cpp
        buffer.h
        buffer_test.cpp)

add_executable(iobuff_selftest
	buffer.cpp
	buffer.h
	iobuffer.h
	iobuffer.cpp
	iobuff_selftest.cpp)
add_executable(iobuff_selftest1
	buffer.cpp
	buffer.h
	iobuffer.h
	iobuffer.cpp
	iobuff_selftest1.cpp)
add_executable(iobuff_othertest
	buffer.cpp
	buffer.h
	iobuffer.h
	iobuffer.cpp
	iobuff_othertest.cpp)
add_executable(iobuff_othertest1
	buffer.cpp
	buffer.h
	iobuffer.h
	iobuffer.cpp
	iobuff_othertest1.cpp)
target_link_libraries(IOBuffer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(iobuff_selftest1 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(iobuff_selftest ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(iobuff_othertest ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(iobuff_othertest1 ${CMAKE_THREAD_LIBS_INIT})
