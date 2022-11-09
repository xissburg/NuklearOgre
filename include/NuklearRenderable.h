#pragma once

#include <CommandBuffer/OgreCbDrawCall.h>
#include <CommandBuffer/OgreCbPipelineStateObject.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreMovableObject.h>
#include <OgreRenderQueue.h>
#include <OgreRenderable.h>
#include <OgreHlmsUnlitDatablock.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreSceneManager.h>
#include <Vao/OgreIndirectBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <cstring>

namespace NuklearOgre
{
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
						  Ogre::SceneManager* sceneManager, Ogre::HlmsManager *hlmsManager,
                          Ogre::uint8 renderQueueId, nk_context *ctx, const nk_convert_config &config)
            : MovableObject(id, objectMemoryManager, sceneManager, renderQueueId)
            , mHlmsManager(hlmsManager)
            , mNuklearCtx(ctx)
            , mNuklearConfig(config)
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

            mIndirectBuffer = vaoManager->createIndirectBuffer(10 * sizeof(Ogre::CbDrawIndexed), Ogre::BT_DYNAMIC_PERSISTENT, 0, false);

            Ogre::Hlms *hlms = mHlmsManager->getHlms(Ogre::HLMS_UNLIT);
            setDatablock(hlms->getDefaultDatablock());

            nk_buffer_init_default(&mCommands);
            nk_buffer_init_default(&mVertexBuffer);
            nk_buffer_init_default(&mElementBuffer);
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
            size_t requiredVertexCount = mVertexBuffer.allocated;

            Ogre::IndexBufferPacked *indexBuffer = mVaoPerLod[0].front()->getIndexBuffer();
            size_t currElemCount = indexBuffer->getNumElements();
            size_t requiredElemCount = mElementBuffer.allocated;

            size_t currCmdCount = mIndirectBuffer->getNumElements();
            size_t requiredCmdCount = mCommands.size;

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

            Ogre::VertexArrayObject *vao = mVaoPerLod[0].front();
            *commandBuffer.addCommand<Ogre::CbVao>() = Ogre::CbVao(vao);
            *commandBuffer.addCommand<Ogre::CbIndirectBuffer>() = Ogre::CbIndirectBuffer(mIndirectBuffer);

            Ogre::Hlms *hlms = mHlmsManager->getHlms(Ogre::HLMS_UNLIT);
            Ogre::QueuedRenderable queuedRenderable(0u, this, this);
            const nk_draw_command *cmd;
            unsigned int offset = 0;

            void *prevTexturePtr = (void *)0xffffffff;
            const Ogre::HlmsCache *hlmsCache = hlms->getMaterial(*lastHlmsCache, passCache, queuedRenderable, false);
            *commandBuffer.addCommand<Ogre::CbPipelineStateObject>() = Ogre::CbPipelineStateObject(&hlmsCache->pso);
            *lastHlmsCache = hlmsCache;

            int baseInstanceAndIndirectBuffers = 0;
            if (vaoManager->supportsIndirectBuffers())
                baseInstanceAndIndirectBuffers = 2;
            else if (vaoManager->supportsBaseInstance())
                baseInstanceAndIndirectBuffers = 1;

            Ogre::CbDrawCallIndexed *drawCall = commandBuffer.addCommand<Ogre::CbDrawCallIndexed>();
            *drawCall = Ogre::CbDrawCallIndexed(baseInstanceAndIndirectBuffers, vao, 0);

            /* iterate over and execute each draw command */
            nk_draw_foreach(cmd, mNuklearCtx, &mCommands)
            {
                if (!cmd->elem_count) continue;

                /* if (cmd->texture.ptr != prevTexturePtr) {
                    prevTexturePtr = cmd->texture.ptr;

                    // Texture changed, add drawcall for all elements processed so far.
                    Ogre::CbDrawCallIndexed *drawCall = commandBuffer.addCommand<Ogre::CbDrawCallIndexed>();

                    Ogre::String name = Ogre::StringConverter::toString(cmd->texture.ptr);
                    Ogre::HlmsDatablock *datablock = hlms->getDatablock(name);

                    if (!datablock) {
                        Ogre::HlmsMacroblock macroblock;
                        macroblock.mDepthCheck = false;
                        macroblock.mDepthWrite = false;
                        datablock = hlms->createDatablock(name, name, macroblock, {}, {});
                        static_cast<Ogre::HlmsUnlitDatablock *>(datablock)->setTexture(0, name);
                    }

                    setDatablock(datablock);

                    const Ogre::HlmsCache *hlmsCache = hlms->getMaterial(*lastHlmsCache, passCache, queuedRenderable, false);
                    *commandBuffer.addCommand<Ogre::CbPipelineStateObject>() = Ogre::CbPipelineStateObject(&hlmsCache->pso);
                    *lastHlmsCache = hlmsCache;
                } */

                drawCall->numDraws++;

                Ogre::CbDrawIndexed *draw = drawCmd++;
                draw->primCount = cmd->elem_count;
                draw->firstVertexIndex = offset;
                draw->instanceCount = 1u;
                draw->baseVertex = 0;
                draw->baseInstance = 0;
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
