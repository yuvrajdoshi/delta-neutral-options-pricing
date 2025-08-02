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
    "${CMAKE_SOURCE_DIR}/src/instruments/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/models/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/strategy/*.cpp"
)

# Add header files
file(GLOB_RECURSE HEADERS 
    "${CMAKE_SOURCE_DIR}/include/core/*.h"
    "${CMAKE_SOURCE_DIR}/include/instruments/*.h"
    "${CMAKE_SOURCE_DIR}/include/models/*.h"
    "${CMAKE_SOURCE_DIR}/include/strategy/*.h"
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

# =================================================================
# LAYER 6: PYTHON INTEGRATION SUPPORT
# =================================================================

# Option to build Python bindings
option(BUILD_PYTHON_BINDINGS "Build Python bindings using pybind11" ON)

if(BUILD_PYTHON_BINDINGS)
    # Find Python and pybind11
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 QUIET)
    
    if(pybind11_FOUND)
        message(STATUS "Found pybind11: ${pybind11_VERSION}")
        
        # Create Python extension module
        pybind11_add_module(volatility_arbitrage_cpp 
            src/bindings/binding.cpp
            ${SOURCES}
        )
        
        # Set properties for the Python module
        target_include_directories(volatility_arbitrage_cpp 
            PRIVATE 
                ${CMAKE_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src
        )
        
        # Link against the core library
        target_link_libraries(volatility_arbitrage_cpp PRIVATE volatility_core)
        
        # Compiler-specific options for Python module
        target_compile_definitions(volatility_arbitrage_cpp 
            PRIVATE VERSION_INFO=${PROJECT_VERSION}
        )
        
        # Set output properties
        set_target_properties(volatility_arbitrage_cpp PROPERTIES
            CXX_VISIBILITY_PRESET "hidden"
            VISIBILITY_INLINES_HIDDEN YES
        )
        
        message(STATUS "Python bindings will be built")
    else()
        message(WARNING "pybind11 not found. Python bindings will not be built.")
        message(STATUS "To install pybind11: pip install pybind11")
    endif()
endif()

# =================================================================
# TESTING AND DEVELOPMENT
# =================================================================

# Add subdirectories for tests if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# Export compile commands for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)