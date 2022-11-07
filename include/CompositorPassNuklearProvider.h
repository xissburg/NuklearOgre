#pragma once

#include "Compositor/Pass/OgreCompositorPassProvider.h"

namespace NuklearOgre
{
    class NuklearOgre;

    class CompositorPassNuklearProvider : public Ogre::CompositorPassProvider
    {
    public:
        CompositorPassNuklearProvider(NuklearOgre *nuklearOgre)
            : mNuklearOgre(nuklearOgre)
        {}

        Ogre::CompositorPassDef *addPassDef(Ogre::CompositorPassType passType, Ogre::IdString customId,
									        Ogre::CompositorTargetDef *parentTargetDef,
									        Ogre::CompositorNodeDef *parentNodeDef) override;

        Ogre::CompositorPass *addPass(const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
								      Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef *rtvDef,
								      Ogre::SceneManager *sceneManager) override;

    private:
        NuklearOgre *mNuklearOgre;
    };
}
