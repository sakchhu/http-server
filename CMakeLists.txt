cmake_minimum_required(VERSION 3.25)

project(http1asio
    DESCRIPTION "Simple Http 1.0 server using asio"
)

include(FetchContent)

FetchContent_Declare(asio 
	GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
	GIT_TAG asio-1-36-0)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/include)

FetchContent_Declare(cx
    GIT_REPOSITORY https://github.com/sakchhu/cx.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(cx)

add_library(${PROJECT_NAME})
add_executable(server)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/connection.cpp
        src/response.cpp
        src/request_parser.cpp
        src/request_handler.cpp
        src/server.cpp
    PUBLIC 
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 23
)

target_link_libraries(${PROJECT_NAME} PRIVATE asio cx)

target_sources(server
    PRIVATE
        src/main.cpp
)

set_target_properties(
    server
    PROPERTIES
    CXX_STANDARD 23
)

target_link_libraries(server ${PROJECT_NAME} cx)

target_compile_options(server 
    PRIVATE 
    -Wall
)

include(CTest)
enable_testing()
add_subdirectory(test)