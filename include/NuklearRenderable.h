#pragma once

#include <OgreRenderable.h>
#include <OgreSceneManager.h>
#include <OgreHlmsManager.h>
#include <OgreHlms.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsUnlitDatablock.h>
#include <Vao/OgreBufferPacked.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <Vao/OgreIndexBufferPacked.h>
#include <Vao/OgreIndirectBufferPacked.h>
#include <CommandBuffer/OgreCbDrawCall.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCbPipelineStateObject.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include "HlmsNuklear.h"

namespace NuklearOgre
{
    class NuklearRenderable : public Ogre::Renderable
    {
    public:
        NuklearRenderable(Ogre::SceneManager* sceneManager, Ogre::HlmsManager *hlmsManager,
                          nk_context *ctx, const nk_convert_config &config,
                          const Ogre::VertexElement2Vec &vertexElements)
            : mSceneManager(sceneManager)
            , mHlmsManager(hlmsManager)
            , mNuklearCtx(ctx)
            , mNuklearConfig(config)
            , mVertexElements(vertexElements)
        #ifdef NK_UINT_DRAW_INDEX
            , mIndexType(Ogre::IndexType::IT_32BIT)
        #else
            , mIndexType(Ogre::IndexType::IT_16BIT)
        #endif
        {
            Ogre::VaoManager *vaoManager = mSceneManager->getDestinationRenderSystem()->getVaoManager();
            Ogre::VertexBufferPacked *vertexBuffer = vaoManager->createVertexBuffer(mVertexElements, 10, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            Ogre::IndexBufferPacked *indexBuffer = vaoManager->createIndexBuffer(mIndexType, 10, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            mIndirectBuffer = vaoManager->createIndirectBuffer(sizeof(Ogre::CbDrawIndexed), Ogre::BT_DYNAMIC_DEFAULT, 0, false);

            Ogre::VertexBufferPackedVec vertexBuffers;
            vertexBuffers.push_back(vertexBuffer);
            Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);
            setVao(vao);

            nk_buffer_init_default(&mNkVertexBuffer);
            nk_buffer_init_default(&mNkElementBuffer);
            nk_buffer_init_default(&mCommands);

            setUseIdentityProjection(true);
            setUseIdentityView(true);
        }

        ~NuklearRenderable()
        {
            Ogre::VertexBufferPacked *vertexBuffer = mVaoPerLod[0].front()->getBaseVertexBuffer();
            Ogre::IndexBufferPacked *indexBuffer = mVaoPerLod[0].front()->getIndexBuffer();

            if (vertexBuffer->isCurrentlyMapped())
            {
                vertexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, vertexBuffer->getNumElements());
            }

            if (indexBuffer->isCurrentlyMapped())
            {
                indexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, indexBuffer->getNumElements());
            }

            if (mIndirectBuffer->isCurrentlyMapped())
            {
                mIndirectBuffer->unmap(Ogre::UO_UNMAP_ALL);
            }

            Ogre::VaoManager *vaoManager = mSceneManager->getDestinationRenderSystem()->getVaoManager();
            vaoManager->destroyVertexBuffer(vertexBuffer);
            vaoManager->destroyIndexBuffer(indexBuffer);
            vaoManager->destroyIndirectBuffer(mIndirectBuffer);
            vaoManager->destroyVertexArrayObject(getVao());

            nk_buffer_free(&mNkVertexBuffer);
            nk_buffer_free(&mNkElementBuffer);
            nk_buffer_free(&mCommands);
        }

        void addCommands(Ogre::CommandBuffer &commandBuffer,
                         const Ogre::HlmsCache **lastHlmsCache,
                         const Ogre::HlmsCache &passCache,
                         Ogre::uint32 &lastVaoName,
                         Ogre::uint32 &lastHlmsCacheHash)
        {
            nk_buffer_clear(&mNkVertexBuffer);
            nk_buffer_clear(&mNkElementBuffer);
            nk_convert(mNuklearCtx, &mCommands, &mNkVertexBuffer, &mNkElementBuffer, &mNuklearConfig);

            Ogre::VertexBufferPacked *vertexBuffer = mVaoPerLod[0].front()->getBaseVertexBuffer();
            size_t currVertexCount = vertexBuffer->getNumElements();
            size_t requiredVertexCount = mNuklearCtx->draw_list.vertex_count;

            Ogre::IndexBufferPacked *indexBuffer = mVaoPerLod[0].front()->getIndexBuffer();
            size_t currElemCount = indexBuffer->getNumElements();
            size_t requiredElemCount = mNuklearCtx->draw_list.element_count;

            size_t currCmdCount = mIndirectBuffer->getNumElements() / sizeof(Ogre::CbDrawIndexed);
            size_t requiredCmdCount = mNuklearCtx->draw_list.cmd_count;

            Ogre::VaoManager *vaoManager = mSceneManager->getDestinationRenderSystem()->getVaoManager();
            bool recreateVao = false;

            if (requiredVertexCount > currVertexCount)
            {
                if (vertexBuffer->isCurrentlyMapped()) {
                    vertexBuffer->unmap(Ogre::UO_UNMAP_ALL);
                }

                vaoManager->destroyVertexBuffer(vertexBuffer);
                size_t newVertexCount = std::max(requiredVertexCount, currVertexCount + (currVertexCount >> 1u));
                vertexBuffer = vaoManager->createVertexBuffer(mVertexElements, newVertexCount, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
                recreateVao = true;
            }

            if (requiredElemCount > currElemCount)
            {
                if (indexBuffer->isCurrentlyMapped()) {
                    indexBuffer->unmap(Ogre::UO_UNMAP_ALL);
                }

                vaoManager->destroyIndexBuffer(indexBuffer);
                size_t newElemCount = std::max(requiredElemCount, currElemCount + (currElemCount >> 1));
                indexBuffer = vaoManager->createIndexBuffer(mIndexType, newElemCount, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
                recreateVao = true;
            }

            if (recreateVao)
            {
                Ogre::VertexBufferPackedVec vertexBuffers;
                vertexBuffers.push_back(vertexBuffer);
                vaoManager->destroyVertexArrayObject(mVaoPerLod[0].front());
                Ogre::VertexArrayObject *vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);
                setVao(vao);
                lastVaoName = 0;
            }

            if (requiredCmdCount > currCmdCount)
            {
                vaoManager->destroyIndirectBuffer(mIndirectBuffer);
                size_t newCmdCount = std::max(requiredCmdCount, currCmdCount + (currCmdCount >> 1));
                size_t newCmdSize = newCmdCount * sizeof(Ogre::CbDrawIndexed);
                mIndirectBuffer = vaoManager->createIndirectBuffer(newCmdSize, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            }

            void *vertex = vertexBuffer->map(0, requiredVertexCount);
            std::memcpy(vertex, nk_buffer_memory_const(&mNkVertexBuffer), requiredVertexCount * Ogre::VaoManager::calculateVertexSize(mVertexElements));
            vertexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, requiredVertexCount);

            void *index = indexBuffer->map(0, requiredElemCount);
            std::memcpy(index, nk_buffer_memory_const(&mNkElementBuffer), requiredElemCount * indexBuffer->getBytesPerElement());
            indexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, requiredElemCount);
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

            if (lastVaoName != vao->getVaoName())
            {
                *commandBuffer.addCommand<Ogre::CbVao>() = Ogre::CbVao(vao);
                *commandBuffer.addCommand<Ogre::CbIndirectBuffer>() = Ogre::CbIndirectBuffer(mIndirectBuffer);
                lastVaoName = vao->getVaoName();
            }

            HlmsNuklear *hlms = static_cast<HlmsNuklear *>(mHlmsManager->getHlms(Ogre::HLMS_UNLIT));
            Ogre::QueuedRenderable queuedRenderable(0u, this, nullptr);

            int baseInstanceAndIndirectBuffers = 0;
            if (vaoManager->supportsIndirectBuffers())
                baseInstanceAndIndirectBuffers = 2;
            else if (vaoManager->supportsBaseInstance())
                baseInstanceAndIndirectBuffers = 1;

            Ogre::CbDrawCallIndexed *drawCall = nullptr;

            const nk_draw_command *cmd;
            unsigned int offset = 0;
            size_t indirectBufferOffset = mIndirectBuffer->_getFinalBufferStart();

            /* iterate over and execute each draw command */
            nk_draw_foreach(cmd, mNuklearCtx, &mCommands)
            {
                if (!cmd->elem_count) continue;

                Ogre::HlmsDatablock *datablock = nullptr;

                if (cmd->texture.ptr == 0)
                {
                    datablock = hlms->getDatablock("nuklear_flat");

                    if (!datablock)
                    {
                        Ogre::HlmsMacroblock macroblock;
                        macroblock.mDepthCheck = false;
                        macroblock.mDepthWrite = false;
                        macroblock.mCullMode = Ogre::CULL_NONE;
                        Ogre::HlmsBlendblock blendblock;
                        datablock = hlms->createDatablock("nuklear_flat", "nuklear_flat", macroblock, blendblock, {});
                    }
                }
                else
                {
                    Ogre::TextureGpu *texture = reinterpret_cast<Ogre::TextureGpu *>(cmd->texture.ptr);
                    Ogre::IdString name = texture->getName();
                    datablock = hlms->getDatablock(name);

                    if (!datablock)
                    {
                        Ogre::HlmsMacroblock macroblock;
                        macroblock.mDepthCheck = false;
                        macroblock.mDepthWrite = false;
                        macroblock.mCullMode = Ogre::CULL_NONE;
                        Ogre::HlmsBlendblock blendblock;
                        blendblock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
                        datablock = hlms->createDatablock(name, "nuklear", macroblock, blendblock, {});
                        static_cast<Ogre::HlmsUnlitDatablock *>(datablock)->setTexture(0, texture);
                    }
                }

                setDatablock(datablock);

                lastHlmsCacheHash = (*lastHlmsCache)->hash;
                const Ogre::HlmsCache *hlmsCache = hlms->getMaterial(*lastHlmsCache, passCache, queuedRenderable, false);

                if (lastHlmsCacheHash != hlmsCache->hash)
                {
                    *commandBuffer.addCommand<Ogre::CbPipelineStateObject>() = Ogre::CbPipelineStateObject(&hlmsCache->pso);
                    *lastHlmsCache = hlmsCache;
                }

                // TODO: add scissor command.

                Ogre::uint32 baseInstance = hlms->fillBuffersForNuklear(
                    *lastHlmsCache, static_cast<HlmsNuklearDatablock *>(datablock), lastHlmsCacheHash, &commandBuffer);

                if (drawCall != commandBuffer.getLastCommand()) {
                    drawCall = commandBuffer.addCommand<Ogre::CbDrawCallIndexed>();
                    *drawCall = Ogre::CbDrawCallIndexed(baseInstanceAndIndirectBuffers, vao, reinterpret_cast<void *>(indirectBufferOffset));
                }

                indirectBufferOffset += sizeof(Ogre::CbDrawIndexed);
                drawCall->numDraws++;

                Ogre::CbDrawIndexed *draw = drawCmd++;
                draw->primCount = cmd->elem_count;
                draw->instanceCount = 1u;
                draw->firstVertexIndex = vao->getIndexBuffer()->_getFinalBufferStart() + offset;
                draw->baseVertex = vao->getBaseVertexBuffer()->_getFinalBufferStart();
                draw->baseInstance = baseInstance;
                offset += cmd->elem_count;
            }
            nk_clear(mNuklearCtx);
            nk_buffer_clear(&mCommands);

            if (vaoManager->supportsIndirectBuffers())
            {
                mIndirectBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, requiredCmdCount * sizeof(Ogre::CbDrawIndexed));
            }
        }

        void setVao(Ogre::VertexArrayObject *vao)
        {
            mVaoPerLod[0].clear();
            mVaoPerLod[1].clear();
            mVaoPerLod[0].push_back(vao);
            mVaoPerLod[1].push_back(vao);
        }

        Ogre::VertexArrayObject *getVao()
        {
            return mVaoPerLod[0].front();
        }

        nk_context *getContext()
        {
            return mNuklearCtx;
        }

        const Ogre::LightList& getLights(void) const override
        {
            static const Ogre::LightList dummy;
            return dummy;
        }

        void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass) override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getRenderOperation."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getRenderOperation");
        }

        void getWorldTransforms(Ogre::Matrix4* xform) const override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getWorldTransforms."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getWorldTransforms");
        }

        bool getCastsShadows(void) const override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getCastsShadows."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getCastsShadows");
        }

    private:
        Ogre::SceneManager *mSceneManager;
        Ogre::HlmsManager *mHlmsManager;
        Ogre::IndirectBufferPacked *mIndirectBuffer;
        nk_context *mNuklearCtx;
        nk_buffer mNkVertexBuffer, mNkElementBuffer;
        nk_buffer mCommands;
        nk_convert_config mNuklearConfig;
        const Ogre::IndexType mIndexType;
        const Ogre::VertexElement2Vec mVertexElements;
    };
}
