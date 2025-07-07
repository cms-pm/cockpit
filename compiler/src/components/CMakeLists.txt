cmake_minimum_required(VERSION 3.10)
project(ComponentVM)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for embedded C++ subset
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-exceptions -fno-rtti")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Platform detection
if(DEFINED ENV{QEMU_PLATFORM})
    add_definitions(-DQEMU_PLATFORM)
    message(STATUS "Building for QEMU platform")
elseif(DEFINED ENV{ARDUINO_PLATFORM})
    add_definitions(-DARDUINO_PLATFORM)
    message(STATUS "Building for Arduino platform")
else()
    add_definitions(-DQEMU_PLATFORM)
    message(STATUS "Default: Building for QEMU platform")
endif()

# Component library
add_library(component_vm
    execution_engine.cpp
    memory_manager.cpp
    io_controller.cpp
    component_vm.cpp
)

target_include_directories(component_vm PUBLIC .)

# Test executable
add_executable(component_test component_test.cpp)
target_link_libraries(component_test component_vm)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(component_test rt)  # For clock_gettime
endif()

# Install targets
install(TARGETS component_vm DESTINATION lib)
install(FILES 
    execution_engine.h
    memory_manager.h
    io_controller.h
    component_vm.h
    DESTINATION include
)