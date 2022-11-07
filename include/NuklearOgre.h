#pragma once

#include "NuklearRenderable.h"
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgrePrerequisites.h>
#include <OgreHlmsCommon.h>

namespace NuklearOgre
{
    class NuklearOgre
    {
    public:
        NuklearOgre(Ogre::Root *root);
        void addContext(nk_context *ctx);
        void removeContext(nk_context *ctx);
        NuklearRenderable * getRenderable(nk_context *ctx);

        void render(Ogre::SceneManager *sceneManager);

    private:
        Ogre::Root *mRoot;
        std::vector<NuklearRenderable *> mRenderables;
        Ogre::CommandBuffer mCommandBuffer;
        const Ogre::HlmsCache *mHlmsCache;
    };
}
