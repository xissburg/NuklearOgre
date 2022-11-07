#include "CompositorPassNuklearProvider.h"
#include "CompositorPassNuklear.h"
#include "CompositorPassNuklearDef.h"

namespace NuklearOgre
{
    Ogre::CompositorPassDef *CompositorPassNuklearProvider::addPassDef(
		Ogre::CompositorPassType passType, Ogre::IdString customId,
        Ogre::CompositorTargetDef *parentTargetDef,
        Ogre::CompositorNodeDef *parentNodeDef )
	{
		if (customId == "nuklear")
			return OGRE_NEW CompositorPassNuklearDef(parentTargetDef);

		return 0;
	}

	Ogre::CompositorPass *CompositorPassNuklearProvider::addPass(
        const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
        Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef *rtvDef,
        Ogre::SceneManager *sceneManager)
	{
		const CompositorPassNuklearDef *def = static_cast<const CompositorPassNuklearDef *>(definition);
		return OGRE_NEW CompositorPassNuklear(def, defaultCamera, sceneManager, rtvDef, parentNode, mNuklearOgre);
	}
}
