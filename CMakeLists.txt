cmake_minimum_required(VERSION 3.12.4)

project(NuklearOgre VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

find_package(SDL2 REQUIRED)
find_package(OGRE REQUIRED)
set(NUKLEAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../Nuklear/src)

set(NuklearOgre_SOURCES
    src/NuklearRenderable.cpp
    src/CompositorPassNuklearProvider.cpp
    src/CompositorPassNuklear.cpp
)

add_library(NuklearOgre ${NuklearOgre_SOURCES})

set_target_properties(NuklearOgre PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

target_compile_features(NuklearOgre PUBLIC cxx_std_11)

target_include_directories(NuklearOgre PUBLIC
    include
)

target_include_directories(NuklearOgre SYSTEM
    PRIVATE ${SDL2_INCLUDE_DIR}
    PRIVATE ${OGRE_INCLUDE_DIRS}
    PRIVATE ${NUKLEAR_INCLUDE_DIR}
)

target_link_libraries(NuklearOgre
    ${SDL2_LIBRARY}
    ${OGRE_HlmsUnlit_LIBRARIES}
    ${OGRE_HlmsPbs_LIBRARIES}
    ${OGRE_RenderSystem_GL3Plus_LIBRARIES}
    ${OGRE}
)
