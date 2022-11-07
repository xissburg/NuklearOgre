#pragma once

#include <Compositor/Pass/OgreCompositorPassDef.h>

namespace NuklearOgre
{
    class CompositorPassNuklearDef : public Ogre::CompositorPassDef
    {
    public:
        CompositorPassNuklearDef(Ogre::CompositorTargetDef *parentTargetDef)
            : Ogre::CompositorPassDef(Ogre::PASS_CUSTOM, parentTargetDef)
        {}
    };
}
