#pragma once

#include <OgreSceneManager.h>

namespace NuklearOgre
{
    class NuklearRenderer
    {
    public:
        virtual void render(Ogre::SceneManager *sceneManager) = 0;
        virtual ~NuklearRenderer() {}
    };
}
