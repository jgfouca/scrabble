cmake_minimum_required(VERSION 3.3)

project(ScrabbleAI CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(kokkos)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wreturn-type -Wswitch-enum -Wsign-compare -Wfloat-equal -Wparentheses -Wreorder -Wshadow -Wold-style-cast -Wextra -D_GLIBCXX_DEBUG")

# Cmake options
set(MEM_DEBUG FALSE CACHE BOOL "Enable memory sanitizing. Requires Gcc/clang. May require setting LD_PRELOAD to <path>/libasan.so (default False)")

if (MEM_DEBUG)
  set(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")
endif()

# Find python3
find_package(PythonLibs 3 REQUIRED)

message("Found python area: ${PYINCLD}")
file(GLOB LIB_SRCS "*.cpp")
add_library(scrabble SHARED ${LIB_SRCS})
target_link_libraries(scrabble PUBLIC kokkos ${PYTHON_LIBRARIES})
target_include_directories(scrabble SYSTEM PUBLIC ${PYTHON_INCLUDE_DIRS})

set(DICTS MEGA_WORD_LIST.txt SIMPLE_WORD_LIST.txt TWL06.txt WWF.txt)
foreach(DICT ${DICTS})
  configure_file(${CMAKE_SOURCE_DIR}/dictionaries/${DICT} ${DICT} COPYONLY)
endforeach()

enable_testing()
include(CTest)

add_subdirectory(tests)
