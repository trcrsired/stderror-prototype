cmake_minimum_required(VERSION 3.10)
project(Win32TestProject)

set(CMAKE_CXX_STANDARD 23)

# Define the shared library
add_library(win32_domain SHARED win32_domain.cc)

# Define the executable and link to the shared library
add_executable(test test.cc)

# Ensure runtime linking by linking to the shared library
target_link_libraries(test PRIVATE win32_domain)

# Set output directories (optional)
set_target_properties(win32_domain PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

