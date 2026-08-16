cmake_minimum_required(VERSION 3.20)
project(ZeroEngineVK LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# 1. External Packages
# -----------------------------------------------------------------------------
find_package(Vulkan REQUIRED)

# -----------------------------------------------------------------------------
# 3. Target Include Directories
# -----------------------------------------------------------------------------
set(DEP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")

add_subdirectory(src)