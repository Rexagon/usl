cmake_minimum_required(VERSION 3.8.0)
project(usl VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_DIR "src")

include_directories(${SOURCE_DIR} include)

set(SOURCES
	"${SOURCE_DIR}/Lexer.cpp"
    "${SOURCE_DIR}/main.cpp"
	"${SOURCE_DIR}/Position.cpp"
)

add_executable(usl ${SOURCES})