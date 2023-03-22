#pragma once

#include <Math/Array/OgreObjectMemoryManager.h>
#include <OgrePrerequisites.h>
#include <TutorialGameState.h>
#include <memory>

struct nk_context;

namespace NuklearOgre
{
    class NuklearRenderer;
}

namespace Demo
{
    class NuklearOgreGameState : public TutorialGameState
    {
    public:
        NuklearOgreGameState(const Ogre::String &helpDescription);

        void createScene01(void) override;

        void update(float timeSinceLast) override;

        void mouseMoved(const SDL_Event &arg) override;
        void mousePressed(const SDL_MouseButtonEvent &arg, Ogre::uint8 id) override;
        void mouseReleased(const SDL_MouseButtonEvent &arg, Ogre::uint8 id) override;

        void textEditing(const SDL_TextEditingEvent& arg) override;
        void textInput(const SDL_TextInputEvent& arg) override;
        void keyPressed(const SDL_KeyboardEvent &arg) override;
        void keyReleased(const SDL_KeyboardEvent &arg) override;

        NuklearOgre::NuklearRenderer *mNuklearRenderer;
        nk_context *mNuklearCtx;
        Ogre::Real mWindowScale;
    };
}
