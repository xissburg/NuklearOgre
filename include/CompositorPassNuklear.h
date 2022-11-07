#pragma once

#include "NuklearRenderable.h"
#include <Compositor/Pass/OgreCompositorPass.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreHlmsCommon.h>
#include <OgrePrerequisites.h>

namespace NuklearOgre
{
    class CompositorPassNuklear : public Ogre::CompositorPass
    {
    public:
        CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                              Ogre::CompositorNode *parentNode,
                              Ogre::HlmsManager *hlmsManager);

		void execute(const Ogre::Camera *lodCamera) override;

    private:
        std::vector<NuklearRenderable *> mRenderables;
        Ogre::CommandBuffer mCommandBuffer;
        Ogre::HlmsManager *mHlmsManager;
        Ogre::HlmsCache mHlmsCache;
    };
}
