# Nuklear-Ogre

Ogre-Next backend for Nuklear immediate-mode GUI.

# Dependencies

- [Ogre-Next](https://github.com/OGRECave/ogre-next)
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)
- [SDL 2](https://github.com/libsdl-org/SDL)

# Usage

This is a header-only library made of two files: `NuklearItem.h` and `NuklearRenderable.h`. You have to invoke `#include <NuklearItem.h>` right after including `nuklear.h` with the desired build options (see [NuklearOgreGameState.cpp](https://github.com/xissburg/NuklearOgre/blob/main/demo/NuklearOgreGameState.cpp)).

`NuklearOgre::NuklearItem` is an `Ogre::Movable` which, much like `Ogre::Item`, can be attached to a `Ogre::SceneNode` and placed anywhere in the world, which means the UI can be presented in a virtual screen. To present it as an overlay, a separate rendering pass with an orthographic camera is necessary. The pass should have a limited range of render queues and the gui items should be assigned to these queues. This ensures the gui item will be drawn in that render pass only. The orthographic camera must be updated when the window size changes using `Ogre::Camera::setOrthoWindow` and the scene node that holds the gui item must be positioned halfway to the top and left so it's centered on screen and its Z position must be set beyond the frustum near plane.

# Build Demo

It assumes Ogre and Nuklear sources are located in the same directory as this project. Otherwise the `NUKLEAR_INCLUDE_DIR` and `OGRE_SAMPLES_DIR` CMake variables will have to be set manually. The build process is the usual `mkdir build; cd build; cmake ..; make`.

You might have to set `CMAKE_MODULE_PATH` to the directory which contains `FindOGRE.cmake` among others, such as `/usr/local/lib/OGRE/cmake`.
