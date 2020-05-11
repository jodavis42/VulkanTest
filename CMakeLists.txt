################################################################################
# Copyright (c) 2018 Josh Davis
################################################################################

################################################################################
# Set required version of cmake and any version specific policies
################################################################################
cmake_minimum_required(VERSION 3.9.1 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

################################################################################
# Declare project(s) and set used languages
################################################################################
project(VulkanFramework CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # -std=c++11 instead of -std=gnu++11

################################################################################
# Set up global variables
################################################################################

#paths
set(SupportedConfigs $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)

set(platform "Windows")
set(platform_library "Windows")

include(CMake/Precompiled.cmake)
include(CMake/CommonCompileOptions.cmake)

add_definitions(-DCOMPILER_MICROSOFT=1)
add_definitions(-DPLATFORM_WINDOWS=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="Win" -D_UNICODE -DUNICODE)

################################################################################
# Call CMakeLists.txt in source folders, adding them to the project non-globally
################################################################################
set(ResourcesDir ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
set(ShaderCoreDir ${ResourcesDir}/ShaderCore)
# external libraries
add_subdirectory(External)
add_subdirectory(Libraries)
add_subdirectory(ZilchShaders)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Application) 

