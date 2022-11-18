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
        NuklearOgre(Ogre::Root *root, Ogre::SceneManager *sceneManager, const nk_convert_config &config)
            : mRoot(root)
            , mSceneManager(sceneManager)
            , mNuklearConfig(config)
        {
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(UIVertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            mNuklearConfig.vertex_layout = vertex_layout;
            mNuklearConfig.vertex_size = sizeof(UIVertex);
            mNuklearConfig.vertex_alignment = NK_ALIGNOF(UIVertex);

            mVertexElements.reserve(4);
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_POSITION ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES ));
            mVertexElements.push_back(Ogre::VertexElement2( Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE ));
        }

        void addContext(nk_context *ctx)
        {
            mRenderables.emplace_back(
                new NuklearRenderable(mSceneManager, mRoot->getHlmsManager(), ctx, mNuklearConfig, mVertexElements));
        }

        void removeContext(nk_context *ctx)
        {
           for (size_t i = 0; i < mRenderables.size(); ++i)
           {
                if (mRenderables[i]->getContext() == ctx)
                {
                    mRenderables[i] = std::move(mRenderables.back());
                    mRenderables.pop_back();
                    break;
                }
           }
        }

        void render(Ogre::SceneManager *sceneManager) override
        {
            mCommandBuffer.setCurrentRenderSystem(sceneManager->getDestinationRenderSystem());

            Ogre::Hlms *hlms = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
            Ogre::HlmsCache passCache = hlms->preparePassHash(0, false, false, sceneManager);
            Ogre::uint32 lastVaoName = 0;
            Ogre::uint32 lastHlmsCacheHash = 0;
            const Ogre::HlmsCache *lastHlmsCache = &c_dummyCache;

            for (size_t i = 0; i < mRenderables.size(); ++i)
            {
                mRenderables[i]->addCommands(mCommandBuffer, &lastHlmsCache, passCache, lastVaoName, lastHlmsCacheHash);
            }

            hlms->preCommandBufferExecution(&mCommandBuffer);

            mCommandBuffer.execute();

            hlms->postCommandBufferExecution(&mCommandBuffer);
        }

    private:
        Ogre::Root *mRoot;
        Ogre::SceneManager *mSceneManager;
        std::vector<std::unique_ptr<NuklearRenderable>> mRenderables;
        Ogre::CommandBuffer mCommandBuffer;
        nk_convert_config mNuklearConfig;
        Ogre::VertexElement2Vec mVertexElements;
    };
}
