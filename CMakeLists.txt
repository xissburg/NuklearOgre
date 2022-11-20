cmake_minimum_required(VERSION 3.12.4)

project(NuklearOgre VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

find_package(OGRE REQUIRED)
set(NUKLEAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../Nuklear)

set(NuklearOgre_SOURCES
    include/CompositorPassNuklear.h
    include/CompositorPassNuklearDef.h
    include/CompositorPassNuklearProvider.h
    include/NuklearOgre.h
    include/NuklearRenderable.h
)

add_library(NuklearOgre INTERFACE)

target_compile_features(NuklearOgre INTERFACE cxx_std_11)

target_include_directories(NuklearOgre INTERFACE
    include
    ${OGRE_INCLUDE_DIRS}
    ${NUKLEAR_INCLUDE_DIR}
)

add_subdirectory(demo)
