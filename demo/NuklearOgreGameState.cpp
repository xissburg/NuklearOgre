#include "NuklearOgreGameState.h"
#include <GraphicsSystem.h>
#include <Math/Array/OgreObjectMemoryManager.h>
#include <OgreCommon.h>
#include <OgreGpuResource.h>
#include <OgreImage2.h>
#include <OgreRoot.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgreWindow.h>
#include <TutorialGameState.h>
#include <OgreCamera.h>
#include <CameraController.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include <nuklear.h>
#include "NuklearItem.h"

namespace Demo
{
    NuklearOgreGameState::NuklearOgreGameState(const Ogre::String &helpDescription)
        : TutorialGameState(helpDescription)
    {

    }

    void nk_sdl_font_stash_begin(nk_font_atlas *atlas)
    {
        nk_font_atlas_init_default(atlas);
        nk_font_atlas_begin(atlas);
    }

    void nk_sdl_font_stash_end(nk_font_atlas *atlas, nk_context *ctx,
                               Ogre::TextureGpuManager *textureManager,
                               nk_draw_null_texture *texNull)
    {
        const void *image; int w, h;
        image = nk_font_atlas_bake(atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

        Ogre::Image2 *imagePtr = new Ogre::Image2;
        imagePtr->loadDynamicImage(const_cast<void *>(image), w, h, 1, Ogre::TextureTypes::Type2D, Ogre::PFG_RGBA8_UNORM_SRGB, false, 1);

        Ogre::TextureGpu *texture = textureManager->createTexture("FontAtlas", Ogre::GpuPageOutStrategy::Discard,
                                                                  Ogre::TextureFlags::AutomaticBatching |
                                                                  Ogre::TextureFlags::PrefersLoadingFromFileAsSRGB,
                                                                  Ogre::TextureTypes::Type2D);
        texture->scheduleTransitionTo(Ogre::GpuResidency::Resident, imagePtr, true);

        nk_font_atlas_end(atlas, nk_handle_ptr(texture), texNull);

        if (atlas->default_font)
            nk_style_set_font(ctx, &atlas->default_font->handle);
    }

    void NuklearOgreGameState::createScene01(void)
    {
        mNuklearCtx.reset(new nk_context);
        mFontAtlas.reset(new nk_font_atlas);
        mTexNull.reset(new nk_draw_null_texture);
        nk_init_default(mNuklearCtx.get(), 0);

        /* Load Fonts: if none of these are loaded a default font will be used  */
        /* Load Cursor: if you uncomment cursor loading please hide the cursor */
        nk_font_atlas *atlas = mFontAtlas.get();
        nk_sdl_font_stash_begin(atlas);
        /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
        /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
        /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
        /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
        /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
        /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
        nk_context *ctx = mNuklearCtx.get();
        Ogre::TextureGpuManager *textureManager = mGraphicsSystem->getRoot()->getHlmsManager()->getRenderSystem()->getTextureGpuManager();
        nk_sdl_font_stash_end(atlas, ctx, textureManager, mTexNull.get());
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        /*nk_style_set_font(ctx, &roboto->handle);*/

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::ObjectMemoryManager *memManager = &sceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC);
        mNuklearItem.reset(new NuklearOgre::NuklearItem(memManager, sceneManager,
                                                        mGraphicsSystem->getRoot()->getHlmsManager(), 200u));
        mNuklearItem->setTexNull(*mTexNull.get());

        mNuklearNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        mNuklearNode->attachObject(mNuklearItem.get());
        mNuklearNode->setPosition(0, 0, -101);
        mNuklearNode->setScale(1, -1, 1);

        mCameraController = new CameraController( mGraphicsSystem, false );

        mGuiCamera = sceneManager->findCamera("GuiCamera");

        TutorialGameState::createScene01();
    }

    void NuklearOgreGameState::destroyScene(void)
    {

    }

    void NuklearOgreGameState::update(float timeSinceLast)
    {
        auto width = mGraphicsSystem->getRenderWindow()->getTexture()->getWidth();
        auto height = mGraphicsSystem->getRenderWindow()->getTexture()->getHeight();
        mGuiCamera->setOrthoWindow(width, height);

        mNuklearNode->setPosition(-Ogre::Real(width)/2, Ogre::Real(height)/2, -110);

        nk_context *ctx = mNuklearCtx.get();

        nk_colorf bg;
        bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                printf("button pressed!\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        mNuklearItem->render(ctx);

        TutorialGameState::update(timeSinceLast);
    }

    void NuklearOgreGameState::mouseMoved(const SDL_Event &arg)
    {
        TutorialGameState::mouseMoved(arg);
    }
    void NuklearOgreGameState::mousePressed(const SDL_MouseButtonEvent &arg, Ogre::uint8 id)
    {
        TutorialGameState::mousePressed(arg, id);
    }
    void NuklearOgreGameState::mouseReleased(const SDL_MouseButtonEvent &arg, Ogre::uint8 id)
    {
        TutorialGameState::mouseReleased(arg, id);
    }

    void NuklearOgreGameState::textEditing(const SDL_TextEditingEvent& arg)
    {

    }
    void NuklearOgreGameState::textInput(const SDL_TextInputEvent& arg)
    {
        TutorialGameState::textInput(arg);
    }
    void NuklearOgreGameState::keyPressed(const SDL_KeyboardEvent &arg)
    {
        TutorialGameState::keyPressed(arg);
    }
    void NuklearOgreGameState::keyReleased(const SDL_KeyboardEvent &arg)
    {
        TutorialGameState::keyReleased(arg);
    }
}
