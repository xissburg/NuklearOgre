#include "NuklearOgreGameState.h"
#include <GraphicsSystem.h>
#include <Math/Array/OgreObjectMemoryManager.h>
#include <OgreCommon.h>
#include <OgreGpuResource.h>
#include <OgreRoot.h>
#include <OgreWindow.h>
#include <SDL_events.h>
#include <TutorialGameState.h>
#include <OgreCamera.h>
#include <CameraController.h>
#include <climits>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>
#include <NuklearRenderer.h>

#include "../../Nuklear/demo/common/overview.c"

#define GUI_RENDER_QUEUE_ID 200u

namespace Demo
{
    NuklearOgreGameState::NuklearOgreGameState(const Ogre::String &helpDescription)
        : TutorialGameState(helpDescription)
    {

    }

    void NuklearOgreGameState::createScene01(void)
    {
        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();

        nk_input_begin(mNuklearCtx);
    }

    void NuklearOgreGameState::update(float timeSinceLast)
    {
        nk_context *ctx = mNuklearCtx;

        nk_input_end(ctx);

        static nk_colorf bg {0.10f, 0.18f, 0.24f, 1.0f};

        if (nk_begin(ctx, "Demo", nk_rect(0, 0, 230, 250),
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

        overview(ctx);

        TutorialGameState::update(timeSinceLast);

        nk_input_begin(ctx);
    }

    void NuklearOgreGameState::mouseMoved(const SDL_Event &evt)
    {
        nk_context *ctx = mNuklearCtx;

        if (evt.type == SDL_MOUSEMOTION)
        {
            if (ctx->input.mouse.grabbed) {
                int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
                nk_input_motion(ctx, x + evt.motion.xrel, y + evt.motion.yrel);
            }
            else {
                nk_input_motion(ctx, evt.motion.x, evt.motion.y);
            }
        }
        else if (evt.type == SDL_MOUSEWHEEL)
        {
            const Uint8* state = SDL_GetKeyboardState(0);
            if (state[SDL_SCANCODE_LSHIFT])
                nk_input_scroll(ctx,nk_vec2((float)evt.wheel.y,(float)evt.wheel.x));
            else
                nk_input_scroll(ctx,nk_vec2((float)evt.wheel.x,(float)evt.wheel.y));
        }

        TutorialGameState::mouseMoved(evt);
    }

    void handleMouseButton(const SDL_MouseButtonEvent &button, bool down, nk_context *ctx)
    {
        const int x = button.x, y = button.y;
        switch(button.button)
        {
            case SDL_BUTTON_LEFT:
                if (button.clicks > 1) {
                    nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
                }
                nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
                break;
            case SDL_BUTTON_MIDDLE:
                nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
                break;
            case SDL_BUTTON_RIGHT:
                nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
                break;
        }
    }

    void NuklearOgreGameState::mousePressed(const SDL_MouseButtonEvent &button, Ogre::uint8 id)
    {
        nk_context *ctx = mNuklearCtx;
        handleMouseButton(button, true, ctx);

        TutorialGameState::mousePressed(button, id);
    }
    void NuklearOgreGameState::mouseReleased(const SDL_MouseButtonEvent &button, Ogre::uint8 id)
    {
        nk_context *ctx = mNuklearCtx;
        handleMouseButton(button, false, ctx);

        TutorialGameState::mouseReleased(button, id);
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
