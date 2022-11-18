#pragma once

#include "Compositor/Pass/OgreCompositorPassProvider.h"
#include "CompositorPassNuklear.h"
#include "CompositorPassNuklearDef.h"

namespace NuklearOgre
{
    class NuklearRenderer;

    class CompositorPassNuklearProvider : public Ogre::CompositorPassProvider
    {
    public:
        CompositorPassNuklearProvider(NuklearRenderer *renderer)
            : mRenderer(renderer)
        {}

        Ogre::CompositorPassDef *addPassDef(
            Ogre::CompositorPassType passType, Ogre::IdString customId,
            Ogre::CompositorTargetDef *parentTargetDef,
            Ogre::CompositorNodeDef *parentNodeDef) override
        {
            if (customId == "nuklear")
                return OGRE_NEW CompositorPassNuklearDef(parentTargetDef);

            return 0;
        }

        Ogre::CompositorPass *addPass(
            const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
            Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef *rtvDef,
            Ogre::SceneManager *sceneManager) override
        {
            const CompositorPassNuklearDef *def = static_cast<const CompositorPassNuklearDef *>(definition);
            return OGRE_NEW CompositorPassNuklear(def, defaultCamera, sceneManager, rtvDef, parentNode, mRenderer);
        }

    private:
        NuklearRenderer *mRenderer;
    };
}
