#pragma once

#include <Compositor/Pass/OgreCompositorPass.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgrePrerequisites.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include <nuklear.h>

namespace NuklearOgre
{
    class CompositorPassNuklear : public Ogre::CompositorPass
    {
    public:
        CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                              Ogre::CompositorNode *parentNode,
                              Ogre::VaoManager *vaoManager);

		void execute(const Ogre::Camera *lodCamera) override;

    private:
        nk_context *mNuklearCtx;
        nk_buffer mCommands;
        nk_buffer mVertexBuffer, mElementBuffer;
        Ogre::VaoManager *mVaoManager;
		Ogre::VertexArrayObject	*mVao;
        Ogre::IndirectBufferPacked *mIndirectBuffer;
        Ogre::CommandBuffer mCommandBuffer;
    };
}
