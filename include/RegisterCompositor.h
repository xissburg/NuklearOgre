#pragma once

#include <OgreRoot.h>
#include "CompositorPassNuklearProvider.h"

namespace NuklearOgre
{
    class NuklearRenderer;

    void RegisterCompositor(Ogre::Root *root, NuklearRenderer *renderer)
    {
        CompositorPassNuklearProvider *compoProvider = OGRE_NEW CompositorPassNuklearProvider(renderer);
            Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
            compositorManager->setCompositorPassProvider(compoProvider);
    }
}
