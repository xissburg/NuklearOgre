#include "NuklearOgre.h"
#include "CompositorPassNuklearProvider.h"
#include <OgreHlms.h>
#include <OgreHlmsCommon.h>
#include <OgreHlmsManager.h>
#include <OgreRoot.h>
#include <Compositor/OgreCompositorManager2.h>

namespace NuklearOgre
{
	static const Ogre::HlmsCache c_dummyCache(0, Ogre::HLMS_MAX, Ogre::HlmsPso());

    NuklearOgre::NuklearOgre(Ogre::Root *root)
        : mRoot(root)
        , mHlmsCache(&c_dummyCache)
    {
        CompositorPassNuklearProvider *compoProvider = OGRE_NEW CompositorPassNuklearProvider(this);
        Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
        compositorManager->setCompositorPassProvider(compoProvider);
    }

    void NuklearOgre::render(Ogre::SceneManager *sceneManager)
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
}
