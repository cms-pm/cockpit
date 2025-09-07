cmake_minimum_required(VERSION 3.20)

project(Canopy 
    VERSION 1.0.0
    DESCRIPTION "CockpitVM Bytecode Uploader GUI"
    LANGUAGES CXX)

# C++20 required for std::expected and modern RAII patterns
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    SerialPort)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shared_libs
)

# Oracle shared library integration
set(ORACLE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tests/oracle_bootloader/lib)
include_directories(${ORACLE_LIB_DIR})

# Source files
set(SOURCES
    src/main/main.cpp
    src/gui/MainWindow.cpp
    src/gui/MainWindow.ui
    src/protocol/ProtocolClient.cpp
    src/protocol/SerialConnection.cpp
    src/hardware/DeviceDiscovery.cpp
)

# Header files
set(HEADERS
    include/gui/MainWindow.h
    include/protocol/ProtocolClient.h
    include/protocol/SerialConnection.h
    include/hardware/DeviceDiscovery.h
    shared_libs/frame_parser/UniversalFrameParser.h
)

# Create executable
add_executable(canopy ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(canopy
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(canopy PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(canopy PRIVATE DEBUG=1)
    target_compile_options(canopy PRIVATE -g -O0)
else()
    target_compile_options(canopy PRIVATE -O2 -DNDEBUG)
endif()

# Install target
install(TARGETS canopy
    RUNTIME DESTINATION bin)

# Testing support
enable_testing()
add_subdirectory(tests)