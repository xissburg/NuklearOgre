#pragma once

#include <OgreHlmsUnlitDatablock.h>
namespace NuklearOgre
{
    class HlmsNuklearDatablock : public Ogre::HlmsUnlitDatablock
    {
        friend class HlmsNuklear;

    public:
        HlmsNuklearDatablock(Ogre::IdString name, Ogre::HlmsUnlit *creator,
							 const Ogre::HlmsMacroblock *macroblock,
							 const Ogre::HlmsBlendblock *blendblock,
							 const Ogre::HlmsParamVec &params ) :
			HlmsUnlitDatablock( name, creator, macroblock, blendblock, params )
		{
		}
    };
}
