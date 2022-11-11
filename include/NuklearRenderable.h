#pragma once

#include <OgreRenderable.h>
#include <OgreMovableObject.h>

namespace NuklearOgre
{
    class NuklearRenderable : public Ogre::Renderable
    {
    public:
        NuklearRenderable(Ogre::MovableObject *movable = 0)
            : mMovable(movable)
        {
            //setUseIdentityProjection(true);
            //setUseIdentityView(true);
        }

        virtual ~NuklearRenderable()
        {

        }

        void setVao(Ogre::VertexArrayObject *vao)
        {
            mVaoPerLod[0].clear();
            mVaoPerLod[1].clear();
            mVaoPerLod[0].push_back(vao);
            mVaoPerLod[1].push_back(vao);
        }

        Ogre::VertexArrayObject *getVao() {
            return mVaoPerLod[0].front();
        }
/*
        bool getUseIdentityWorldMatrix(void) const override
        {
            return true;
        } */

        const Ogre::LightList& getLights(void) const override
        {
            return mMovable->queryLights();
        }

        void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass) override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getRenderOperation."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getRenderOperation");
        }

        void getWorldTransforms(Ogre::Matrix4* xform) const override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getWorldTransforms."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getWorldTransforms");
        }

        bool getCastsShadows(void) const override
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
                            "NuklearRenderable does not implement getCastsShadows."
                            " You've put a v2 object in "
                            "the wrong RenderQueue ID (which is set to be compatible with "
                            "v1::Entity). Do not mix v2 and v1 objects",
                            "NuklearRenderable::getCastsShadows");
        }

        Ogre::MovableObject *mMovable;
    };
}
