cmake_minimum_required(VERSION 3.3)

project(ScrabbleAI CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(kokkos)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wreturn-type -Wswitch-enum -Wsign-compare -Wfloat-equal -Wparentheses -Wreorder -Wshadow -Wold-style-cast -Wextra -D_GLIBCXX_DEBUG")

# Find python3 include path
execute_process(COMMAND python3 -c "from sysconfig import get_paths as gp; print(gp()['include'])"
  OUTPUT_VARIABLE PYINCLD
  ERROR_VARIABLE PYERRS
  RESULT_VARIABLE SUCCESS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT SUCCESS STREQUAL "0")
  message(FATAL_ERROR "Failed to get python3 include dirs: ${PYERRS}")
endif()

message("Found python area: ${PYINCLD}")
file(GLOB LIB_SRCS "*.cpp")
add_library(scrabble SHARED ${LIB_SRCS})
target_link_libraries(scrabble PUBLIC kokkos)
target_include_directories(scrabble SYSTEM PUBLIC ${PYINCLD})

set(DICTS MEGA_WORD_LIST.txt SIMPLE_WORD_LIST.txt TWL06.txt WWF.txt)
foreach(DICT ${DICTS})
  configure_file(${CMAKE_SOURCE_DIR}/dictionaries/${DICT} ${DICT} COPYONLY)
endforeach()

enable_testing()
include(CTest)

add_subdirectory(tests)
