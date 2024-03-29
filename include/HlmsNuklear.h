#pragma once

#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <OgreDescriptorSetTexture.h>
#include <OgreHlmsListener.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsUnlitDatablock.h>
#include <OgreMatrix4.h>
#include <OgreQuaternion.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreTexBufferPacked.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <CommandBuffer/OgreCbTexture.h>
#include "HlmsNuklearDatablock.h"

namespace NuklearOgre
{
    class NuklearRenderable;

    class HlmsNuklear : public Ogre::HlmsUnlit
    {
    public:
        HlmsNuklear(Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders, float windowScale)
            : HlmsUnlit(dataFolder, libraryFolders)
            , mWindowScaleInv(1.f / windowScale)
        {}

        HlmsNuklear(Ogre::Archive *dataFolder, Ogre::ArchiveVec *libraryFolders,
                    Ogre::HlmsTypes type, const Ogre::String &typeName, float windowScale)
            : HlmsUnlit(dataFolder, libraryFolders, type, typeName)
            , mWindowScaleInv(1.f / windowScale)
        {}

        virtual ~HlmsNuklear() = default;

        void calculateHashForPreCreate(Ogre::Renderable *renderable, Ogre::PiecesMap *inOutPieces) override
        {
            Ogre::HlmsUnlit::calculateHashForPreCreate(renderable, inOutPieces);

            if (!renderable->getUserObjectBindings().getUserAny("nuklear").isEmpty())
            {
                setProperty("nuklear", 1);
                setProperty(Ogre::HlmsBaseProp::PsoClipDistances, 4);
            }
        }

        Ogre::uint32 fillBuffersForNuklear(const Ogre::HlmsCache *cache,
                                           const HlmsNuklearDatablock *datablock,
                                           Ogre::uint32 lastCacheHash,
                                           Ogre::CommandBuffer *commandBuffer,
                                           const Ogre::Vector4 &clipRect)
        {
            if (OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH(lastCacheHash) != mType)
            {
                //We changed HlmsType, rebind the shared textures.
                mLastDescTexture = 0;
                mLastDescSampler = 0;
                mLastBoundPool = 0;

                //layout(binding = 0) uniform PassBuffer {} pass
                Ogre::ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
                *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                    Ogre::CbShaderBuffer(Ogre::VertexShader, 0, passBuffer, 0, passBuffer->getTotalSizeBytes());
                *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                    Ogre::CbShaderBuffer(Ogre::PixelShader, 0, passBuffer, 0, passBuffer->getTotalSizeBytes());

                //layout(binding = 2) uniform InstanceBuffer {} instance
                if( mCurrentConstBuffer < mConstBuffers.size() &&
                    (size_t)((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <=
                        mCurrentConstBufferSize )
                {
                    *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                            Ogre::CbShaderBuffer(Ogre::VertexShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0);
                    *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                            Ogre::CbShaderBuffer(Ogre::PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0);
                }

                rebindTexBuffer(commandBuffer);

                mListener->hlmsTypeChanged(false, commandBuffer, datablock, 0u);
            }

            //Don't bind the material buffer on caster passes (important to keep
            //MDI & auto-instancing running on shadow map passes)
            if (mLastBoundPool != datablock->getAssignedPool())
            {
                //layout(binding = 1) uniform MaterialBuf {} materialArray
                const ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
                *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                    Ogre::CbShaderBuffer(Ogre::VertexShader, 1, newPool->materialBuffer, 0, newPool->materialBuffer->getTotalSizeBytes());
                *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                    Ogre::CbShaderBuffer(Ogre::PixelShader, 1, newPool->materialBuffer, 0, newPool->materialBuffer->getTotalSizeBytes());
                if( newPool->extraBuffer )
                {
                    Ogre::TexBufferPacked *extraBuffer = static_cast<Ogre::TexBufferPacked *>( newPool->extraBuffer );
                    *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
                        Ogre::CbShaderBuffer(Ogre::VertexShader, 1, extraBuffer, 0, extraBuffer->getTotalSizeBytes());
                }

                mLastBoundPool = newPool;
            }

            Ogre::uint32 * RESTRICT_ALIAS currentMappedConstBuffer    = mCurrentMappedConstBuffer;
            float * RESTRICT_ALIAS currentMappedTexBuffer       = mCurrentMappedTexBuffer;

            Ogre::TextureGpu *renderTarget = mRenderSystem->getCurrentRenderViewports()[0].getCurrentTarget();

            Ogre::Vector3 position = Ogre::Vector3(-1, 1, 0);
            Ogre::Vector3 scale = Ogre::Vector3(2.0 / (renderTarget->getWidth() * mWindowScaleInv),
                                               -2.0 / (renderTarget->getHeight() * mWindowScaleInv), 1);
            Ogre::Matrix4 worldMat = Ogre::Matrix4::IDENTITY;
            worldMat.makeTransform(position, scale, Ogre::Quaternion::IDENTITY);

            bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4) >
                                                                                    mCurrentConstBufferSize;

            // Tex buffer includes world matrix and clip rect.
            const size_t minimumTexBufferSize = 16 + 4;
            bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                        minimumTexBufferSize >= mCurrentTexBufferSize;

            if( exceedsConstBuffer || exceedsTexBuffer )
            {
                currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                if( exceedsTexBuffer )
                    mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                else
                    rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                currentMappedTexBuffer = mCurrentMappedTexBuffer;
            }

            //---------------------------------------------------------------------------
            //                          ---- VERTEX SHADER ----
            //---------------------------------------------------------------------------
            bool useIdentityProjection = true;

            //uint materialIdx[]
            *currentMappedConstBuffer = datablock->getAssignedSlot();
            *reinterpret_cast<float * RESTRICT_ALIAS>( currentMappedConstBuffer + 1 ) =
                datablock->mShadowConstantBias * mConstantBiasScale;
            *(currentMappedConstBuffer+2) = useIdentityProjection;
            currentMappedConstBuffer += 4;

            //mat4 worldViewProj
            Ogre::Matrix4 tmp =
                mPreparedPass.viewProjMatrix[mUsingInstancedStereo ? 4u : useIdentityProjection] * worldMat;
    #if !OGRE_DOUBLE_PRECISION
            memcpy( currentMappedTexBuffer, &tmp, sizeof( Ogre::Matrix4 ) );
            currentMappedTexBuffer += 16;
    #else
            for( int y = 0; y < 4; ++y )
            {
                for( int x = 0; x < 4; ++x )
                {
                    *currentMappedTexBuffer++ = tmp[ y ][ x ];
                }
            }
    #endif

            //vec4 clipRect
            memcpy(currentMappedTexBuffer, &clipRect, sizeof(Ogre::Vector4));
            currentMappedTexBuffer += 4;

            //---------------------------------------------------------------------------
            //                          ---- PIXEL SHADER ----
            //---------------------------------------------------------------------------

            if (datablock->mTexturesDescSet != mLastDescTexture)
            {
                //Bind textures
                size_t texUnit = mTexUnitSlotStart;

                if (datablock->mTexturesDescSet)
                {
                    *commandBuffer->addCommand<Ogre::CbTextures>() =
                        Ogre::CbTextures(texUnit, std::numeric_limits<Ogre::uint16>::max(), datablock->mTexturesDescSet);

                    if (!mHasSeparateSamplers)
                    {
                        *commandBuffer->addCommand<Ogre::CbSamplers>() =
                            Ogre::CbSamplers(texUnit, datablock->mSamplersDescSet);
                    }

                    texUnit += datablock->mTexturesDescSet->mTextures.size();
                }

                mLastDescTexture = datablock->mTexturesDescSet;
            }

            if (datablock->mSamplersDescSet != mLastDescSampler && mHasSeparateSamplers)
            {
                if (datablock->mSamplersDescSet)
                {
                    //Bind samplers
                    size_t texUnit = mSamplerUnitSlotStart;
                    *commandBuffer->addCommand<Ogre::CbSamplers>() =
                            Ogre::CbSamplers(texUnit, datablock->mSamplersDescSet);
                    mLastDescSampler = datablock->mSamplersDescSet;
                }
            }

            mCurrentMappedConstBuffer   = currentMappedConstBuffer;
            mCurrentMappedTexBuffer     = currentMappedTexBuffer;

            return ((mCurrentMappedConstBuffer - mStartMappedConstBuffer) >> 2) - 1;
        }

    private:
        float mWindowScaleInv;
    };
}
