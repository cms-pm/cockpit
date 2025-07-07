cmake_minimum_required(VERSION 3.15)
project(ArduinoCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use system ANTLR4 packages
find_program(ANTLR4_JAR antlr4 REQUIRED)
find_package(antlr4-runtime REQUIRED)

# Generate parser from grammar
set(GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/grammar/ArduinoC.g4)
set(ANTLR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/antlr4_generated)

# Custom command to generate ANTLR4 files
add_custom_command(
    OUTPUT
        ${ANTLR_OUTPUT_DIR}/ArduinoCBaseListener.cpp
        ${ANTLR_OUTPUT_DIR}/ArduinoCBaseListener.h
        ${ANTLR_OUTPUT_DIR}/ArduinoCBaseVisitor.cpp
        ${ANTLR_OUTPUT_DIR}/ArduinoCBaseVisitor.h
        ${ANTLR_OUTPUT_DIR}/ArduinoCLexer.cpp
        ${ANTLR_OUTPUT_DIR}/ArduinoCLexer.h
        ${ANTLR_OUTPUT_DIR}/ArduinoCParser.cpp
        ${ANTLR_OUTPUT_DIR}/ArduinoCParser.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ANTLR_OUTPUT_DIR}
    COMMAND ${ANTLR4_JAR} -Dlanguage=Cpp -visitor -o ${ANTLR_OUTPUT_DIR} ${GRAMMAR_FILE}
    DEPENDS ${GRAMMAR_FILE}
    COMMENT "Generating ANTLR4 parser"
)

# Include directories
include_directories(${ANTLR_OUTPUT_DIR})
include_directories(/usr/include/antlr4-runtime)
include_directories(src/components)

# Compiler executable
add_executable(arduino_compiler
    src/main.cpp
    src/bytecode_visitor.cpp
    src/symbol_table.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseListener.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseVisitor.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCLexer.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCParser.cpp
)

# Link libraries
target_link_libraries(arduino_compiler antlr4-runtime)

# Test executable
add_executable(test_compiler
    tests/test_parser.cpp
    src/bytecode_visitor.cpp
    src/symbol_table.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseListener.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseVisitor.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCLexer.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCParser.cpp
)

target_link_libraries(test_compiler antlr4-runtime)

# Add ComponentVM library subdirectory
add_subdirectory(src/components)

# Simple 32-bit instruction format test
add_executable(simple_integration_test
    src/simple_integration_test.cpp
    src/bytecode_visitor.cpp
    src/symbol_table.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseListener.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCBaseVisitor.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCLexer.cpp
    ${ANTLR_OUTPUT_DIR}/ArduinoCParser.cpp
)

target_link_libraries(simple_integration_test antlr4-runtime)

# Comprehensive test runner
add_executable(test_runner
    tests/test_runner.c
)

target_include_directories(test_runner PRIVATE tests/)