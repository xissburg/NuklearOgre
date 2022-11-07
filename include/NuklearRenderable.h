#pragma once

#include <CommandBuffer/OgreCbDrawCall.h>
#include <CommandBuffer/OgreCbPipelineStateObject.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreMovableObject.h>
#include <OgreRenderQueue.h>
#include <OgreRenderable.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreSceneManager.h>
#include <Vao/OgreIndirectBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <cstring>

namespace NuklearOgre
{
    struct UIVertex {
        float position[2];
        float uv[2];
        nk_byte col[4];
    };

    class NuklearRenderable : public Ogre::Renderable, public Ogre::MovableObject
    {
        void setVao(Ogre::VertexArrayObject *vao)
        {
            mVaoPerLod[0].clear();
            mVaoPerLod[1].clear();
            mVaoPerLod[0].push_back(vao);
            mVaoPerLod[1].push_back(vao);
        }

    public:
        NuklearRenderable(Ogre::IdType id, Ogre::ObjectMemoryManager *objectMemoryManager,
						  Ogre::SceneManager* manager, Ogre::uint8 renderQueueId)
            : MovableObject(id, objectMemoryManager, manager, renderQueueId)
        #ifdef NK_UINT_DRAW_INDEX
            , mIndexType(Ogre::IndexType::IT_32BIT)
        #else
            , mIndexType(Ogre::IndexType::IT_16BIT)
        #endif
        {
            Ogre::Aabb aabb(Ogre::Aabb::BOX_INFINITE);
            mObjectData.mLocalAabb->setFromAabb(aabb, mObjectData.mIndex);
            mObjectData.mWorldAabb->setFromAabb(aabb, mObjectData.mIndex);
            mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Ogre::Real>::max();
            mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Ogre::Real>::max();

            setDatablock(mHlmsManager->getHlms(Ogre::HLMS_UNLIT)->getDefaultDatablock());

            setUseIdentityProjection(true);
            setUseIdentityView(true);

            // Add this renderable into the movable.
            mRenderables.push_back(this);

            mVertexElements.reserve(4);
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_POSITION ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_USHORT2_NORM, Ogre::VES_TEXTURE_COORDINATES ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE ));

            Ogre::VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
            Ogre::VertexBufferPacked *vertexBuffer = vaoManager->createVertexBuffer(mVertexElements, 10, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
            Ogre::IndexBufferPacked *indexBuffer = vaoManager->createIndexBuffer(mIndexType, 10, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);

            Ogre::VertexBufferPackedVec vertexBuffers;
            vertexBuffers.push_back(vertexBuffer);
            Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);
            setVao(vao);

            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(UIVertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&mNuklearConfig, 0, sizeof(mNuklearConfig));
            mNuklearConfig.vertex_layout = vertex_layout;
            mNuklearConfig.vertex_size = sizeof(UIVertex);
            mNuklearConfig.vertex_alignment = NK_ALIGNOF(UIVertex);
            //mNuklearConfig.tex_null = dev->tex_null;
            mNuklearConfig.circle_segment_count = 22;
            mNuklearConfig.curve_segment_count = 22;
            mNuklearConfig.arc_segment_count = 22;
            mNuklearConfig.global_alpha = 1.0f;
            mNuklearConfig.shape_AA = NK_ANTI_ALIASING_OFF;
            mNuklearConfig.line_AA = NK_ANTI_ALIASING_OFF;
        }

        virtual ~NuklearRenderable()
        {

        }

        void addCommands(Ogre::CommandBuffer &commandBuffer,
                         const Ogre::HlmsCache **lastHlmsCache,
                         const Ogre::HlmsCache &passCache)
        {
            nk_buffer_clear(&mVertexBuffer);
            nk_buffer_clear(&mElementBuffer);
            nk_convert(mNuklearCtx, &mCommands, &mVertexBuffer, &mElementBuffer, &mNuklearConfig);

            Ogre::VertexBufferPacked *vertexBuffer = mVaoPerLod[0].front()->getBaseVertexBuffer();
            size_t currVertexCount = vertexBuffer->getNumElements();
            size_t requiredVertexCount = nk_buffer_total(&mVertexBuffer);

            Ogre::IndexBufferPacked *indexBuffer = mVaoPerLod[0].front()->getIndexBuffer();
            size_t currElemCount = indexBuffer->getNumElements();
            size_t requiredElemCount = nk_buffer_total(&mElementBuffer);

            size_t currCmdCount = mIndirectBuffer->getNumElements();
            size_t requiredCmdCount = nk_buffer_total(&mCommands);

            Ogre::VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
            bool recreateVao = false;

            if (requiredVertexCount > currVertexCount)
            {
                size_t newVertexCount = std::max(requiredVertexCount, currVertexCount + (currVertexCount >> 1u));
                vaoManager->destroyVertexBuffer(vertexBuffer);
                vertexBuffer = vaoManager->createVertexBuffer(mVertexElements, newVertexCount, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
                recreateVao = true;
            }

            if (requiredElemCount > currElemCount)
            {
                size_t newElemCount = std::max(requiredElemCount, currElemCount + (currElemCount >> 1));
                vaoManager->destroyIndexBuffer(indexBuffer);
                indexBuffer = vaoManager->createIndexBuffer(mIndexType, newElemCount, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
                recreateVao = true;
            }

            if (requiredCmdCount > currCmdCount)
            {
                size_t newCmdCount = std::max(requiredCmdCount, currCmdCount + (currCmdCount >> 1));
                vaoManager->destroyIndirectBuffer(mIndirectBuffer);
                size_t newCmdSize = newCmdCount * sizeof(Ogre::CbDrawIndexed);
                mIndirectBuffer = vaoManager->createIndirectBuffer(newCmdSize, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
            }

            if (recreateVao)
            {
                Ogre::VertexBufferPackedVec vertexBuffers;
                vertexBuffers.push_back(vertexBuffer);
                vaoManager->destroyVertexArrayObject(mVaoPerLod[0].front());
                Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);
                setVao(vao);
            }

            void *vertex = vertexBuffer->map(0, requiredVertexCount);
            std::memcpy(vertex, nk_buffer_memory_const(&mVertexBuffer), requiredVertexCount);
            vertexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT, 0u, requiredVertexCount);

            void *index = indexBuffer->map(0, requiredElemCount);
            std::memcpy(index, nk_buffer_memory_const(&mElementBuffer), requiredElemCount);
            indexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT, 0u, requiredElemCount);
            Ogre::CbDrawIndexed *drawCmd;

            if (vaoManager->supportsIndirectBuffers())
            {
                drawCmd = reinterpret_cast<Ogre::CbDrawIndexed *>(mIndirectBuffer->map(0, requiredCmdCount * sizeof(Ogre::CbDrawIndexed)));
            }
            else
            {
                drawCmd = reinterpret_cast<Ogre::CbDrawIndexed *>(mIndirectBuffer->getSwBufferPtr());
            }

            Ogre::Hlms *hlms = mHlmsManager->getHlms(Ogre::HLMS_UNLIT);
            Ogre::QueuedRenderable queuedRenderable(0u, this, this);

            const Ogre::HlmsCache *hlmsCache = hlms->getMaterial(*lastHlmsCache, passCache, queuedRenderable, false);

            if ((*lastHlmsCache)->hash != hlmsCache->hash)
            {
                *commandBuffer.addCommand<Ogre::CbPipelineStateObject>() = Ogre::CbPipelineStateObject(&hlmsCache->pso);
                *lastHlmsCache = hlmsCache;
            }

            Ogre::VertexArrayObject *vao = mVaoPerLod[0].front();
            *commandBuffer.addCommand<Ogre::CbVao>() = Ogre::CbVao(vao);
            *commandBuffer.addCommand<Ogre::CbIndirectBuffer>() = Ogre::CbIndirectBuffer(mIndirectBuffer);

            const nk_draw_command *cmd;
            unsigned int offset = 0;

            /* iterate over and execute each draw command */
            nk_draw_foreach(cmd, mNuklearCtx, &mCommands)
            {
                if (!cmd->elem_count) continue;

                /* glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
                glScissor((GLint)(cmd->clip_rect.x * scale.x),
                    (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                    (GLint)(cmd->clip_rect.w * scale.x),
                    (GLint)(cmd->clip_rect.h * scale.y)); */
                Ogre::CbDrawIndexed *drawCall = drawCmd++;
                drawCall->primCount = cmd->elem_count;
                drawCall->firstVertexIndex = offset;
                drawCall->instanceCount = 1u;
                drawCall->baseVertex = 0;
                drawCall->baseInstance = 0;
                offset += cmd->elem_count;
            }
            nk_clear(mNuklearCtx);
            nk_buffer_clear(&mCommands);

            if (vaoManager->supportsIndirectBuffers())
            {
                mIndirectBuffer->unmap(Ogre::UO_KEEP_PERSISTENT, 0u, requiredCmdCount * sizeof(Ogre::CbDrawIndexed));
            }
        }

        nk_context *getContext() {
            return mNuklearCtx;
        }

        //Overrides from MovableObject
        const Ogre::String& getMovableType(void) const
        {
            return Ogre::BLANKSTRING;
        }

		//Overrides from Renderable
        const Ogre::LightList& getLights(void) const
        {
            return this->queryLights(); //Return the data from our MovableObject base class.
        }

        void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass)
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getRenderOperation."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getRenderOperation");
        }

        void getWorldTransforms(Ogre::Matrix4* xform) const
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getWorldTransforms."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getWorldTransforms");
        }

        bool getCastsShadows(void) const
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getCastsShadows."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getCastsShadows");
        }

    private:
        nk_context *mNuklearCtx;
        nk_convert_config mNuklearConfig;
        nk_buffer mCommands;
        nk_buffer mVertexBuffer, mElementBuffer;
        Ogre::HlmsManager *mHlmsManager;
        Ogre::IndirectBufferPacked *mIndirectBuffer;
        Ogre::VertexElement2Vec mVertexElements;
        const Ogre::IndexType mIndexType;
        Ogre::CommandBuffer mCommandBuffer;
    };
}
