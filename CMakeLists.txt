cmake_minimum_required(VERSION 3.16)
project(VolatilityArbitrageSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
)

# Add header files
file(GLOB_RECURSE HEADERS 
    "${CMAKE_SOURCE_DIR}/include/core/*.h"
)

# Create a library for the core components
add_library(volatility_core ${SOURCES} ${HEADERS})

# Set target include directories
target_include_directories(volatility_core 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
)

# Compiler-specific options
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(volatility_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Add subdirectories for tests if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# Export compile commands for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)