cmake_minimum_required(VERSION 3.3)

project(ScrabbleAI CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wreturn-type -Wswitch-enum -Wsign-compare -Wfloat-equal -Wparentheses -Wreorder -Wshadow -Wold-style-cast -Wextra -D_GLIBCXX_DEBUG")

file(GLOB LIB_SRCS "*.cpp")
add_library(libscrabble ${LIB_SRCS})

add_executable(scrabble main.C)
target_link_libraries(scrabble libscrabble)

set(DICTS MEGA_WORD_LIST.txt SIMPLE_WORD_LIST.txt TWL06.txt WWF.txt)
foreach(DICT ${DICTS})
  configure_file(${CMAKE_SOURCE_DIR}/dictionaries/${DICT} ${DICT} COPYONLY)
endforeach()
