cmake_minimum_required(VERSION 3.10)
project(JobReport)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories("./include")

# Check GCC version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if(GCC_VERSION VERSION_LESS 9)
        set(ADDITIONAL_LIBS stdc++)
    endif()
endif()

# Add the executable
add_executable(jobreport ./src/main.cpp)

# Link libraries
target_link_libraries(jobreport dcgm ${ADDITIONAL_LIBS})

