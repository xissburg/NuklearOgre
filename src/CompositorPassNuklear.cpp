#include "CompositorPassNuklear.h"
#include "NuklearRenderable.h"
#include "NuklearOgre.h"
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreHlmsManager.h>
#include <OgreHlms.h>
#include <nuklear.h>
#include <cstring>

namespace NuklearOgre
{
    CompositorPassNuklear::CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                              					 Ogre::Camera *defaultCamera,
                              					 Ogre::SceneManager *sceneManager,
                              					 const Ogre::RenderTargetViewDef *rtv,
                              					 Ogre::CompositorNode *parentNode,
                              					 NuklearOgre *nuklearOgre)
        : Ogre::CompositorPass(definition, parentNode)
		, mSceneManager(sceneManager)
		, mCamera(defaultCamera)
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

		mSceneManager->_setCamerasInProgress(Ogre::CamerasInProgress(mCamera));
		mSceneManager->_setCurrentCompositorPass(this);

		notifyPassPreExecuteListeners();

		mNuklearOgre->render(mSceneManager);

		mSceneManager->_setCurrentCompositorPass(0);

		notifyPassPosExecuteListeners();

		profilingEnd();
	}
}
