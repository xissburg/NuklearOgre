#pragma once

#include "CompositorPassNuklearProvider.h"
#include "NuklearRenderable.h"
#include "NuklearRenderer.h"
#include <CommandBuffer/OgreCommandBuffer.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsCommon.h>
#include <OgreRoot.h>
#include <OgreHlms.h>
#include <OgreSceneManager.h>

namespace NuklearOgre
{
    static const Ogre::HlmsCache c_dummyCache(0, Ogre::HLMS_MAX, Ogre::HlmsPso());

    struct UIVertex {
        float position[2];
        float uv[2];
        nk_byte col[4];
    };

    void RegisterCompositor(Ogre::Root *root, NuklearRenderer *renderer)
    {
        CompositorPassNuklearProvider *compoProvider = OGRE_NEW CompositorPassNuklearProvider(renderer);
            Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
            compositorManager->setCompositorPassProvider(compoProvider);
    }

    class NuklearOgre : public NuklearRenderer
    {
    public:
        NuklearOgre(Ogre::Root *root, Ogre::SceneManager *sceneManager)
            : mRoot(root)
            , mSceneManager(sceneManager)
            , mHlmsCache(&c_dummyCache)
        {
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

            mVertexElements.reserve(4);
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_POSITION ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE ));
        }

        void addContext(nk_context *ctx)
        {
            NuklearRenderable *renderable = OGRE_NEW NuklearRenderable(
                Ogre::Id::generateNewId<NuklearRenderable>(), &mObjectMemoryManager,
                mSceneManager, mRoot->getHlmsManager(), 0u, ctx, mNuklearConfig, mVertexElements);
            mRenderables.push_back(renderable);
        }

        void removeContext(nk_context *ctx)
        {
           for (size_t i = 0; i < mRenderables.size(); ++i) {
                if (mRenderables[i]->getContext() == ctx) {
                    OGRE_DELETE mRenderables[i];
                    mRenderables[i] = mRenderables.back();
                    mRenderables.pop_back();
                    break;
                }
           }
        }

        NuklearRenderable *getRenderable(nk_context *ctx)
        {
            for (size_t i = 0; i < mRenderables.size(); ++i) {
                if (mRenderables[i]->getContext() == ctx) {
                    return mRenderables[i];
                }
            }

            return NULL;
        }

        void render(Ogre::SceneManager *sceneManager) override
        {
            mCommandBuffer.setCurrentRenderSystem(sceneManager->getDestinationRenderSystem());

            Ogre::Hlms *hlms = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
            Ogre::HlmsCache passCache = hlms->preparePassHash(0, false, false, sceneManager);
            Ogre::uint32 lastVaoName = 0;
            Ogre::uint32 lastHlmsCacheHash = 0;

            for (size_t i = 0; i < mRenderables.size(); ++i) {
                mRenderables[i]->addCommands(mCommandBuffer, &mHlmsCache, passCache, lastVaoName, lastHlmsCacheHash);
            }

            hlms->preCommandBufferExecution(&mCommandBuffer);

            mCommandBuffer.execute();

            hlms->postCommandBufferExecution(&mCommandBuffer);
        }

        void setTexNull(const nk_draw_null_texture &texNull)
        {
            mNuklearConfig.tex_null = texNull;
        }

    private:
        Ogre::Root *mRoot;
        Ogre::SceneManager *mSceneManager;
        std::vector<NuklearRenderable *> mRenderables;
        Ogre::CommandBuffer mCommandBuffer;
        const Ogre::HlmsCache *mHlmsCache;
		Ogre::ObjectMemoryManager mObjectMemoryManager;
        nk_convert_config mNuklearConfig;
        Ogre::VertexElement2Vec mVertexElements;
    };
}
