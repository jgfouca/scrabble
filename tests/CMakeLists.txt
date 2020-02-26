
file(GLOB SRCS "*.cpp")
add_executable(scrabble_tests ${SRCS})

target_include_directories(scrabble_tests PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(scrabble_tests scrabble)

add_test(NAME full_test_1 COMMAND ./tests/scrabble_tests test_one   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME full_test_2 COMMAND ./tests/scrabble_tests test_two   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME full_test_3 COMMAND ./tests/scrabble_tests test_three WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME full_test_4 COMMAND ./tests/scrabble_tests test_four  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME full_test_5 COMMAND ./tests/scrabble_tests test_five  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test-files/*")
foreach(TEST_FILE ${TEST_FILES})
  get_filename_component(TEST_FILE_BASE "${TEST_FILE}" NAME)
  configure_file("${TEST_FILE}" "${CMAKE_BINARY_DIR}/test-files/${TEST_FILE_BASE}" COPYONLY)
endforeach()
