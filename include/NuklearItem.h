#pragma once

#include "NuklearRenderable.h"
#include <OgreMovableObject.h>
#include <OgreSceneManager.h>
#include <OgreHlmsManager.h>
#include <OgreHlms.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsUnlitDatablock.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <cstring>

namespace NuklearOgre
{
    struct UIVertex {
        float position[2];
        float uv[2];
        nk_byte col[4];
    };

    class NuklearItem : public Ogre::MovableObject
    {
    public:
        NuklearItem(Ogre::ObjectMemoryManager *objectMemoryManager,
					Ogre::SceneManager* sceneManager, Ogre::HlmsManager *hlmsManager,
                    Ogre::uint8 renderQueueId = 0u)
            : MovableObject(Ogre::Id::generateNewId<NuklearOgre::NuklearItem>(), objectMemoryManager, sceneManager, renderQueueId)
            , mHlmsManager(hlmsManager)
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

            mVertexElements.reserve(4);
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_POSITION ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE ));

            Ogre::VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();
            mVertexBuffer = vaoManager->createVertexBuffer(mVertexElements, 10, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            mIndexBuffer = vaoManager->createIndexBuffer(mIndexType, 10, Ogre::BT_DYNAMIC_DEFAULT, 0, false);

            nk_buffer_init_default(&mNkVertexBuffer);
            nk_buffer_init_default(&mNkElementBuffer);
            nk_buffer_init_default(&mCommands);

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
            mNuklearConfig.circle_segment_count = 22;
            mNuklearConfig.curve_segment_count = 22;
            mNuklearConfig.arc_segment_count = 22;
            mNuklearConfig.global_alpha = 1.0f;
            mNuklearConfig.shape_AA = NK_ANTI_ALIASING_OFF;
            mNuklearConfig.line_AA = NK_ANTI_ALIASING_OFF;
        }

        void render(nk_context *ctx)
        {
            nk_buffer_clear(&mNkVertexBuffer);
            nk_buffer_clear(&mNkElementBuffer);
            nk_convert(ctx, &mCommands, &mNkVertexBuffer, &mNkElementBuffer, &mNuklearConfig);

            size_t currVertexCount = mVertexBuffer->getNumElements();
            size_t requiredVertexCount = ctx->draw_list.vertex_count;

            size_t currElemCount = mIndexBuffer->getNumElements();
            size_t requiredElemCount = ctx->draw_list.element_count;

            Ogre::VaoManager *vaoManager = mManager->getDestinationRenderSystem()->getVaoManager();

            if (requiredVertexCount > currVertexCount)
            {
                size_t newVertexCount = std::max(requiredVertexCount, currVertexCount + (currVertexCount >> 1u));
                vaoManager->destroyVertexBuffer(mVertexBuffer);
                mVertexBuffer = vaoManager->createVertexBuffer(mVertexElements, newVertexCount, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            }

            if (requiredElemCount > currElemCount)
            {
                size_t newElemCount = std::max(requiredElemCount, currElemCount + (currElemCount >> 1));
                vaoManager->destroyIndexBuffer(mIndexBuffer);
                mIndexBuffer = vaoManager->createIndexBuffer(mIndexType, newElemCount, Ogre::BT_DYNAMIC_DEFAULT, 0, false);
            }

            void *vertex = mVertexBuffer->map(0, requiredVertexCount);
            std::memcpy(vertex, nk_buffer_memory_const(&mNkVertexBuffer), requiredVertexCount * Ogre::VaoManager::calculateVertexSize(mVertexElements));
            mVertexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, requiredVertexCount);

            void *index = mIndexBuffer->map(0, requiredElemCount);
            std::memcpy(index, nk_buffer_memory_const(&mNkElementBuffer), requiredElemCount * (mIndexType == Ogre::IT_16BIT ? 2 : 4));
            mIndexBuffer->unmap(Ogre::UO_UNMAP_ALL, 0u, requiredElemCount);

            Ogre::VertexBufferPackedVec vertexBuffers;
            vertexBuffers.push_back(mVertexBuffer);

            Ogre::Hlms *hlms = mHlmsManager->getHlms(Ogre::HLMS_UNLIT);

            const nk_draw_command *cmd;
            unsigned int offset = 0;
            unsigned int cmdIndex = 0;

            /* iterate over and execute each draw command */
            nk_draw_foreach(cmd, ctx, &mCommands)
            {
                if (!cmd->elem_count) continue;

                NuklearRenderable *renderable;
                Ogre::VertexArrayObject *vao;

                if (cmdIndex < mNkRenderables.size())
                {
                    renderable = mNkRenderables[cmdIndex].get();
                    vao = renderable->getVao();
                }
                else
                {
                    mNkRenderables.emplace_back(new NuklearRenderable(this));
                    renderable = mNkRenderables.back().get();
                    vao = vaoManager->createVertexArrayObject(vertexBuffers, mIndexBuffer, Ogre::OT_TRIANGLE_LIST);
                    renderable->setVao(vao);
                }

                vao->setPrimitiveRange(offset, cmd->elem_count);

                Ogre::HlmsDatablock *datablock;

                if (cmd->texture.ptr == 0)
                {
                    datablock = hlms->getDefaultDatablock();
                }
                else
                {
                    Ogre::TextureGpu *texture = reinterpret_cast<Ogre::TextureGpu *>(cmd->texture.ptr);
                    Ogre::IdString name = texture->getName();
                    datablock = hlms->getDatablock(name);

                    if (!datablock) {
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

                renderable->setDatablock(datablock);

                offset += cmd->elem_count;
                ++cmdIndex;
            }
            nk_clear(ctx);
            nk_buffer_clear(&mCommands);

            mNkRenderables.erase(mNkRenderables.begin() + cmdIndex, mNkRenderables.end());

            mRenderables.clear();
            mRenderables.reserve(mNkRenderables.size());

            for (size_t i = 0; i < mNkRenderables.size(); ++i) {
                mRenderables.push_back(mNkRenderables[i].get());
            }
        }

        const Ogre::String& getMovableType(void) const override
        {
            return Ogre::BLANKSTRING;
        }

        void setTexNull(const nk_draw_null_texture &texNull)
        {
            mNuklearConfig.tex_null = texNull;
        }

    private:
        Ogre::HlmsManager *mHlmsManager;
        Ogre::VertexBufferPacked *mVertexBuffer;
        Ogre::IndexBufferPacked *mIndexBuffer;
        nk_buffer mNkVertexBuffer, mNkElementBuffer;
        nk_buffer mCommands;
        nk_convert_config mNuklearConfig;
        const Ogre::IndexType mIndexType;
        Ogre::VertexElement2Vec mVertexElements;
        std::vector<std::unique_ptr<NuklearRenderable>> mNkRenderables;
    };
}
