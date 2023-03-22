#pragma once

#include <Compositor/Pass/OgreCompositorPass.h>
#include <OgreSceneManager.h>
#include "NuklearRenderer.h"

namespace NuklearOgre
{
    class CompositorPassNuklear : public Ogre::CompositorPass
    {
    public:
        CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                              Ogre::Camera *defaultCamera,
                              const Ogre::RenderTargetViewDef *rtv,
                              Ogre::CompositorNode *parentNode,
                              NuklearRenderer *renderer)
            : Ogre::CompositorPass(definition, parentNode)
            , mCamera(defaultCamera)
            , mRenderer(renderer)
        {
            initialize(rtv);
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

            Ogre::SceneManager *sceneManager = mRenderer->getSceneManager();
            sceneManager->_setCamerasInProgress(Ogre::CamerasInProgress(mCamera));
            sceneManager->_setCurrentCompositorPass(this);

            notifyPassPreExecuteListeners();

            executeResourceTransitions();
            setRenderPassDescToCurrent();

            mRenderer->render();

            sceneManager->_setCurrentCompositorPass(0);

            notifyPassPosExecuteListeners();

            profilingEnd();
        }

    private:
        NuklearRenderer *mRenderer;
        Ogre::Camera *mCamera;
    };
}
