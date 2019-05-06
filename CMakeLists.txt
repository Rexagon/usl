cmake_minimum_required(VERSION 3.8)
project(usl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_DIR "src")

include_directories(${SOURCE_DIR} include)

set(SOURCES
	"${SOURCE_DIR}/EarleyItem.cpp"
	"${SOURCE_DIR}/Evaluator.cpp"
	"${SOURCE_DIR}/Lexer.cpp"
	"${SOURCE_DIR}/LexerGrammar.cpp"
    "${SOURCE_DIR}/main.cpp"
	"${SOURCE_DIR}/Parser.cpp"
	"${SOURCE_DIR}/ParserGrammar.cpp"
	"${SOURCE_DIR}/Position.cpp"
	"${SOURCE_DIR}/Rules.cpp"
	"${SOURCE_DIR}/Symbol.cpp"
	"${SOURCE_DIR}/ByteCode.cpp"
	"${SOURCE_DIR}/CommandBuffer.cpp"
)

add_executable(usl ${SOURCES})