#include "NuklearOgreGameState.h"
#include <GraphicsSystem.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include <nuklear.h>
#include "NuklearOgre.h"

namespace Demo
{
    NuklearOgreGameState::NuklearOgreGameState(const Ogre::String &helpDescription)
        : TutorialGameState(helpDescription)
    {

    }

    void NuklearOgreGameState::createScene01(void)
    {
        mNuklearOgre.reset(new NuklearOgre::NuklearOgre(mGraphicsSystem->getRoot(), mGraphicsSystem->getSceneManager()));
    }

    void NuklearOgreGameState::destroyScene(void)
    {

    }

    void NuklearOgreGameState::update(float timeSinceLast)
    {

    }

    void NuklearOgreGameState::mouseMoved(const SDL_Event &arg)
    {

    }
    void NuklearOgreGameState::mousePressed(const SDL_MouseButtonEvent &arg, Ogre::uint8 id)
    {

    }
    void NuklearOgreGameState::mouseReleased(const SDL_MouseButtonEvent &arg, Ogre::uint8 id)
    {

    }

    void NuklearOgreGameState::textEditing(const SDL_TextEditingEvent& arg)
    {

    }
    void NuklearOgreGameState::textInput(const SDL_TextInputEvent& arg)
    {

    }
    void NuklearOgreGameState::keyPressed(const SDL_KeyboardEvent &arg)
    {

    }
    void NuklearOgreGameState::keyReleased(const SDL_KeyboardEvent &arg)
    {

    }
}
