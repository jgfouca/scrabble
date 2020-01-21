
file(GLOB SRCS "*.cpp")
add_executable(scrabble_tests ${SRCS})

target_include_directories(scrabble_tests PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(scrabble_tests scrabble)

add_test(NAME full_tests COMMAND ./tests/scrabble_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
