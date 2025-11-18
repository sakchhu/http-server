link_libraries(${PROJECT_NAME})

add_executable(parser-tests parser-tests.cpp)
target_link_libraries(parser-tests PRIVATE ${PROJECT_NAME})

add_test(NAME parser COMMAND parser-tests)