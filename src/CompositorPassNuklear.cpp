#include "CompositorPassNuklear.h"
#include "NuklearRenderable.h"
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreHlmsManager.h>
#include <OgreHlms.h>
#include <nuklear.h>
#include <cstring>

namespace NuklearOgre
{
    CompositorPassNuklear::CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                                                 Ogre::CompositorNode *parentNode,
                                                 Ogre::HlmsManager *hlmsManager)
        : Ogre::CompositorPass(definition, parentNode)
        , mHlmsManager(hlmsManager)
        , mHlmsCache(0, Ogre::HLMS_MAX, Ogre::HlmsPso())
    {
    }

    void CompositorPassNuklear::execute(const Ogre::Camera *lodCamera)
	{
		//Execute a limited number of times?
		if (mNumPassesLeft != std::numeric_limits<Ogre::uint32>::max())
		{
			if (!mNumPassesLeft)
				return;
			--mNumPassesLeft;
		}

		profilingBegin();

		notifyPassEarlyPreExecuteListeners();

		Ogre::SceneManager *sceneManager = lodCamera->getSceneManager();
		sceneManager->_setCamerasInProgress(Ogre::CamerasInProgress(lodCamera));
		sceneManager->_setCurrentCompositorPass(this);

        mCommandBuffer.setCurrentRenderSystem(sceneManager->getDestinationRenderSystem());

		//Fire the listener in case it wants to change anything
		notifyPassPreExecuteListeners();

		Ogre::Hlms *hlms = mHlmsManager->getHlms(Ogre::HLMS_UNLIT);
		Ogre::HlmsCache passCache = hlms->preparePassHash(0, false, false, sceneManager);

        hlms->preCommandBufferExecution(&mCommandBuffer);

        for (size_t i = 0; i < mRenderables.size(); ++i) {
		    mRenderables[i]->addCommands(mCommandBuffer, &mHlmsCache, passCache);
        }

		mCommandBuffer.execute();

		hlms->postCommandBufferExecution(&mCommandBuffer);

		sceneManager->_setCurrentCompositorPass(0);

		notifyPassPosExecuteListeners();

		profilingEnd();
	}
}
