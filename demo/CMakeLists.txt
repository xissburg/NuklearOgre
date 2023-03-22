cmake_minimum_required(VERSION 3.12.4)

project(NuklearOgreDemo VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(SDL2 REQUIRED)
set(NUKLEAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../Nuklear)
set(OGRE_SAMPLES_DIR ${CMAKE_SOURCE_DIR}/../ogre/Samples)

set(OGRE_SAMPLES_SOURCE_DIR "${OGRE_SAMPLES_DIR}/2.0/Common/src")
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1" )

file( GLOB NuklearOgreDemo_SOURCES
    "${OGRE_SAMPLES_SOURCE_DIR}/*.cpp"
    "${OGRE_SAMPLES_SOURCE_DIR}/System/*.cpp"
    "${OGRE_SAMPLES_SOURCE_DIR}/System/Android/*.cpp"
    "${OGRE_SAMPLES_SOURCE_DIR}/Threading/*.cpp"
    "${OGRE_SAMPLES_SOURCE_DIR}/Utils/*.cpp" )

if( OGRE_BUILD_PLATFORM_APPLE_IOS )
	file( GLOB IOS_SOURCE_FILES
		"${OGRE_SAMPLES_SOURCE_DIR}/System/iOS/*.cpp"
		"${OGRE_SAMPLES_SOURCE_DIR}/System/iOS/*.mm" )
	set( NuklearOgreDemo_SOURCES ${NuklearOgreDemo_SOURCES} ${IOS_SOURCE_FILES} )
elseif( APPLE )
    file( GLOB OSX_SOURCE_FILES
        "${OGRE_SAMPLES_SOURCE_DIR}/System/Desktop/*.cpp"
        "${OGRE_SAMPLES_SOURCE_DIR}/System/OSX/*.mm" )
	set( NuklearOgreDemo_SOURCES ${NuklearOgreDemo_SOURCES} ${OSX_SOURCE_FILES} )
elseif( NOT ANDROID )
	file( GLOB DESKTOP_SOURCE_FILES
		"${OGRE_SAMPLES_SOURCE_DIR}/System/Desktop/*.cpp" )
	set( NuklearOgreDemo_SOURCES ${NuklearOgreDemo_SOURCES} ${DESKTOP_SOURCE_FILES} )
endif()

set(NuklearOgreDemo_SOURCES
    ${NuklearOgreDemo_SOURCES}
    NuklearOgreDemo.cpp
    NuklearOgreGameState.cpp
    ${NUKLEAR_INCLUDE_DIR}/demo/common/overview.c
)

if (APPLE)
    add_executable(NuklearOgreDemo MACOSX_BUNDLE ${NuklearOgreDemo_SOURCES})
elseif (WIN32)
    add_executable(NuklearOgreDemo WIN32 ${NuklearOgreDemo_SOURCES})
else ()
    add_executable(NuklearOgreDemo ${NuklearOgreDemo_SOURCES})
endif ()

# Define _DEBUG for Ogre.
target_compile_definitions(NuklearOgreDemo PUBLIC $<$<CONFIG:Debug>:_DEBUG>)

set_target_properties(NuklearOgreDemo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

target_compile_features(NuklearOgreDemo PUBLIC cxx_std_11)

target_include_directories(NuklearOgreDemo PUBLIC
    include
)

target_include_directories(NuklearOgreDemo SYSTEM
    PUBLIC ${SDL2_INCLUDE_DIR}
    PUBLIC ${OGRE_INCLUDE_DIRS}
    PUBLIC ${OGRE_INCLUDE_DIR}/Hlms/Common
    PUBLIC ${OGRE_INCLUDE_DIR}/Atmosphere
    PUBLIC ${OGRE_Overlay_INCLUDE_DIR}
    PUBLIC ${OGRE_HlmsUnlit_INCLUDE_DIR}
    PUBLIC ${OGRE_HlmsPbs_INCLUDE_DIR}
    PUBLIC ${NUKLEAR_INCLUDE_DIR}
    PUBLIC ${OGRE_SAMPLES_DIR}/2.0/Common/include
)

target_link_libraries(NuklearOgreDemo
    NuklearOgre
    OgreAtmosphere
    ${SDL2_LIBRARY}
    ${OGRE_HlmsUnlit_LIBRARIES}
    ${OGRE_HlmsPbs_LIBRARIES}
    ${OGRE_RenderSystem_GL3Plus_LIBRARIES}
    ${OGRE_Overlay_LIBRARIES}
    ${OGRE_LIBRARIES}
)

add_custom_command(TARGET NuklearOgreDemo POST_BUILD
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/demo/resources ${PROJECT_BINARY_DIR}/bin
)
