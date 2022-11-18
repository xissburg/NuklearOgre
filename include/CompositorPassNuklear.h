#pragma once

#include "NuklearRenderer.h"
#include <Compositor/Pass/OgreCompositorPass.h>
#include <OgreSceneManager.h>

namespace NuklearOgre
{
    class NuklearOgre;

    class CompositorPassNuklear : public Ogre::CompositorPass
    {
    public:
        CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                              Ogre::Camera *defaultCamera,
                              Ogre::SceneManager *sceneManager,
                              const Ogre::RenderTargetViewDef *rtv,
                              Ogre::CompositorNode *parentNode,
                              NuklearRenderer *renderer)
            : Ogre::CompositorPass(definition, parentNode)
            , mSceneManager(sceneManager)
            , mCamera(defaultCamera)
            , mRenderer(renderer)
        {
        }

        void execute(const Ogre::Camera *lodCamera) override
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

            mRenderer->render(mSceneManager);

            mSceneManager->_setCurrentCompositorPass(0);

            notifyPassPosExecuteListeners();

            profilingEnd();
        }

    private:
        NuklearRenderer *mRenderer;
        Ogre::SceneManager *mSceneManager;
        Ogre::Camera *mCamera;
    };
}
