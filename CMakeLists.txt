cmake_minimum_required(VERSION 3.10)
project(JobReport)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories("./include")

# Ensure GCC version is at least 9 for std::filesystem support
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if(GCC_VERSION VERSION_LESS 9)
        message(FATAL_ERROR "GCC version must be at least 9 to support std::filesystem")
    endif()
endif()

# Add the executable
add_executable(jobreport ./src/main.cpp)

# Find and link libraries
find_library(DCGM_LIB NAMES dcgm HINTS /usr/lib64)
if(NOT DCGM_LIB)
    message(FATAL_ERROR "libdcgm not found in /usr/lib64")
endif()

# Set RPATH
set_target_properties(jobreport PROPERTIES
    INSTALL_RPATH "/usr/lib64"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Link standard library statically
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

target_link_libraries(jobreport ${DCGM_LIB})
