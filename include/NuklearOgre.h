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

    class NuklearOgre : public NuklearRenderer
    {
    public:
        NuklearOgre(Ogre::Root *root, Ogre::SceneManager *sceneManager)
            : mRoot(root)
            , mSceneManager(sceneManager)
            , mHlmsCache(&c_dummyCache)
        {
            CompositorPassNuklearProvider *compoProvider = OGRE_NEW CompositorPassNuklearProvider(this);
            Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
            compositorManager->setCompositorPassProvider(compoProvider);
        }

        void addContext(nk_context *ctx)
        {
            NuklearRenderable *renderable = OGRE_NEW NuklearRenderable(
                Ogre::Id::generateNewId<NuklearRenderable>(), &mObjectMemoryManager, mSceneManager, 0u);
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

            for (size_t i = 0; i < mRenderables.size(); ++i) {
                mRenderables[i]->addCommands(mCommandBuffer, &mHlmsCache, passCache);
            }

            hlms->preCommandBufferExecution(&mCommandBuffer);

            mCommandBuffer.execute();

            hlms->postCommandBufferExecution(&mCommandBuffer);
        }

    private:
        Ogre::Root *mRoot;
        Ogre::SceneManager *mSceneManager;
        std::vector<NuklearRenderable *> mRenderables;
        Ogre::CommandBuffer mCommandBuffer;
        const Ogre::HlmsCache *mHlmsCache;
		Ogre::ObjectMemoryManager mObjectMemoryManager;
    };
}
