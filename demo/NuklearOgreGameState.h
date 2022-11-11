#pragma once

#include <Math/Array/OgreObjectMemoryManager.h>
#include <OgrePrerequisites.h>
#include <TutorialGameState.h>
#include <memory>

struct nk_context;
struct nk_font_atlas;
struct nk_draw_null_texture;

namespace NuklearOgre
{
    class NuklearItem;
}

namespace Demo
{
    class NuklearOgreGameState : public TutorialGameState
    {
    public:
        NuklearOgreGameState(const Ogre::String &helpDescription);

        void createScene01(void) override;
        void destroyScene(void) override;

        void update(float timeSinceLast) override;

        void mouseMoved(const SDL_Event &arg) override;
        void mousePressed(const SDL_MouseButtonEvent &arg, Ogre::uint8 id) override;
        void mouseReleased(const SDL_MouseButtonEvent &arg, Ogre::uint8 id) override;

        void textEditing(const SDL_TextEditingEvent& arg) override;
        void textInput(const SDL_TextInputEvent& arg) override;
        void keyPressed(const SDL_KeyboardEvent &arg) override;
        void keyReleased(const SDL_KeyboardEvent &arg) override;

    private:
        std::unique_ptr<nk_context> mNuklearCtx;
        std::unique_ptr<nk_font_atlas> mFontAtlas;
        std::unique_ptr<nk_draw_null_texture> mTexNull;
        std::unique_ptr<NuklearOgre::NuklearItem> mNuklearItem;
        Ogre::Camera *mGuiCamera;
        Ogre::SceneNode *mNuklearNode;
    };
}
