#pragma once

#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <Vao/OgreVertexBufferPacked.h>

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
    class NuklearRenderable : public Ogre::Renderable, public Ogre::MovableObject
    {
    public:
        NuklearRenderable(Ogre::IdType id, Ogre::ObjectMemoryManager *objectMemoryManager,
						  Ogre::SceneManager* manager, Ogre::uint8 renderQueueId);
        virtual ~NuklearRenderable();

        void addCommands(Ogre::CommandBuffer &commandBuffer,
                         const Ogre::HlmsCache **lastHlmsCache,
                         const Ogre::HlmsCache &passCache);

        //Overrides from MovableObject
		virtual const Ogre::String& getMovableType(void) const;

		//Overrides from Renderable
		virtual const Ogre::LightList& getLights(void) const;
		virtual void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass);
		virtual void getWorldTransforms(Ogre::Matrix4* xform) const;
		virtual bool getCastsShadows(void) const;

    private:
        nk_context *mNuklearCtx;
        nk_convert_config mNuklearConfig;
        nk_buffer mCommands;
        nk_buffer mVertexBuffer, mElementBuffer;
        Ogre::HlmsManager *mHlmsManager;
		Ogre::VertexArrayObject	*mVao;
        Ogre::IndirectBufferPacked *mIndirectBuffer;
        Ogre::VertexElement2Vec mVertexElements;
        const Ogre::IndexType mIndexType;
        Ogre::CommandBuffer mCommandBuffer;
    };
}
