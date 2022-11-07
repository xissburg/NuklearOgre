#pragma once

#include <Compositor/Pass/OgreCompositorPass.h>
#include <OgrePrerequisites.h>

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
                              NuklearOgre *nuklearOgre);

		void execute(const Ogre::Camera *lodCamera) override;

    private:
        NuklearOgre *mNuklearOgre;
        Ogre::SceneManager *mSceneManager;
        Ogre::Camera *mCamera;
    };
}
