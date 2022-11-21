# Nuklear-Ogre

Ogre-Next backend for Nuklear immediate-mode GUI.

# Dependencies

- [Ogre-Next](https://github.com/OGRECave/ogre-next)
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)
- [SDL 2](https://github.com/libsdl-org/SDL) to build demo

# Usage

This is a header-only library. You have to invoke `#include <NuklearOgre.h>` right after including `nuklear.h` with the desired build options. Make sure you define `NK_IMPLEMENTATION` in a single cpp file before including `nuklear.h` and `NuklearOgre.h` to prevent multiple definition errors.

This implementation uses a slightly custom HLMS so `NuklearOgre::HlmsNuklear` must be registered instead of `Ogre::HlmsUnlit`. It's also important to call `Ogre::Hlms::reloadFrom` to load the custom pieces into the shader. See `NuklearOgreGameState::registerHlms` for details. The custom piece is necessary to perform clipping, e.g. using `gl_ClipDistance` in OpenGL.

The UI is rendered in a custom pass. In the compositor, create an extra pass using `pass custom nuklear`.

See [NuklearOgreDemo.cpp](https://github.com/xissburg/NuklearOgre/blob/main/demo/NuklearOgreDemo.cpp) to learn how to initialize Nuklear and load fonts. Then, a `OgreNuklear::NuklearRenderer` must be created with the Nuklear config and the context must be added to it. Then call `NuklearOgre::RegisterCustomPass` before creating a workspace.

# Build Demo

It assumes Ogre and Nuklear sources are located in the same directory as this project. Otherwise the `NUKLEAR_INCLUDE_DIR` and `OGRE_SAMPLES_DIR` CMake variables will have to be set manually. The build process is the usual `mkdir build; cd build; cmake ..; make`.

You might have to set `CMAKE_MODULE_PATH` to the directory which contains `FindOGRE.cmake` among others, such as `/usr/local/lib/OGRE/cmake`.
