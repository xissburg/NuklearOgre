#include "CompositorPassNuklear.h"
#include <OgreCamera.h>
#include <OgreCommon.h>
#include <OgrePrerequisites.h>
#include <OgreSceneManager.h>
#include <Vao/OgreBufferPacked.h>
#include <Vao/OgreIndexBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreVertexBufferPacked.h>
#include <Vao/OgreIndirectBufferPacked.h>
#include <CommandBuffer/OgreCbDrawCall.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <nuklear.h>
#include <cstring>

namespace NuklearOgre
{
    struct UIVertex {
        float position[2];
        float uv[2];
        nk_byte col[4];
    };

    CompositorPassNuklear::CompositorPassNuklear(const Ogre::CompositorPassDef *definition,
                                                 Ogre::CompositorNode *parentNode,
                                                 Ogre::VaoManager *vaoManager)
        : Ogre::CompositorPass(definition, parentNode)
        , mVaoManager(vaoManager)
    {
        nk_buffer_init_default(&mVertexBuffer);
        nk_buffer_init_default(&mElementBuffer);
    }

    void CompositorPassNuklear::execute(const Ogre::Camera *lodCamera)
	{
		//Execute a limited number of times?
		if (mNumPassesLeft != std::numeric_limits<Ogre::uint32>::max())
		{
			if (!mNumPassesLeft)
				return;
			--mNumPassesLeft;
		}

		profilingBegin();

		notifyPassEarlyPreExecuteListeners();

		Ogre::SceneManager *sceneManager = lodCamera->getSceneManager();
		sceneManager->_setCamerasInProgress(Ogre::CamerasInProgress(lodCamera));
		sceneManager->_setCurrentCompositorPass(this);

        mCommandBuffer.setCurrentRenderSystem(sceneManager->getDestinationRenderSystem());

		//Fire the listener in case it wants to change anything
		notifyPassPreExecuteListeners();

		nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(UIVertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(UIVertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        memset(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(UIVertex);
        config.vertex_alignment = NK_ALIGNOF(UIVertex);
        config.tex_null = dev->tex_null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = NK_ANTI_ALIASING_OFF;
        config.line_AA = NK_ANTI_ALIASING_OFF;

        nk_buffer_clear(&mVertexBuffer);
        nk_buffer_clear(&mElementBuffer);
        nk_convert(mNuklearCtx, &mCommands, &mVertexBuffer, &mElementBuffer, &config);

        Ogre::VertexBufferPacked *vertexBuffer = mVao->getBaseVertexBuffer();
        size_t currVertexCount = vertexBuffer->getNumElements();
        size_t requiredVertexCount = nk_buffer_total(&mVertexBuffer);

        Ogre::IndexBufferPacked *indexBuffer = mVao->getIndexBuffer();
        size_t currElemCount = indexBuffer->getNumElements();
        size_t requiredElemCount = nk_buffer_total(&mElementBuffer);

        bool recreateVao = false;

        if (requiredVertexCount > currVertexCount)
        {
            size_t newVertexCount = std::max(requiredVertexCount, currVertexCount + (currVertexCount >> 1u));
            mVaoManager->destroyVertexBuffer(vertexBuffer);

            Ogre::VertexElement2Vec vertexElements;
            vertexElements.reserve( 4 );
            vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_FLOAT2, Ogre::VES_POSITION ) );
            vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_USHORT2_NORM, Ogre::VES_TEXTURE_COORDINATES ) );
            vertexElements.push_back( Ogre::VertexElement2( Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE ) );

            vertexBuffer = mVaoManager->createVertexBuffer(vertexElements, newVertexCount, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
            recreateVao = true;
        }

        if (requiredElemCount > currElemCount)
        {
            size_t newElemCount = std::max(requiredElemCount, currElemCount + (currElemCount >> 1));
            mVaoManager->destroyIndexBuffer(indexBuffer);

        #ifdef NK_UINT_DRAW_INDEX
            Ogre::IndexType indexType = Ogre::IndexType::IT_32BIT;
        #else
            Ogre::IndexType indexType = Ogre::IndexType::IT_16BIT;
        #endif
            indexBuffer = mVaoManager->createIndexBuffer(indexType, newElemCount, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
            recreateVao = true;
        }

        if (recreateVao)
        {
            mVaoManager->destroyVertexArrayObject(mVao);

            Ogre::VertexBufferPackedVec vertexBuffers;
		    vertexBuffers.push_back(vertexBuffer);
            mVao = mVaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);
        }

        void *vertex = vertexBuffer->map(0, vertexBuffer->getNumElements());
        std::memcpy(vertex, nk_buffer_memory_const(&mVertexBuffer), requiredVertexCount);
        vertexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT, 0u, requiredVertexCount);

        void *index = indexBuffer->map(0, indexBuffer->getNumElements());
        std::memcpy(index, nk_buffer_memory_const(&mElementBuffer), requiredElemCount);
        indexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT, 0u, requiredElemCount);

        size_t currCmdCount = mIndirectBuffer->getNumElements();
        size_t requiredCmdCount = nk_buffer_total(&mCommands);

        if (requiredCmdCount < currCmdCount)
        {
            size_t newCmdCount = std::max(requiredCmdCount, currCmdCount + (currCmdCount >> 1));
            mVaoManager->destroyIndirectBuffer(mIndirectBuffer);
            size_t newCmdSize = newCmdCount * sizeof(Ogre::CbDrawIndexed);
            mIndirectBuffer = mVaoManager->createIndirectBuffer(newCmdSize, Ogre::BT_DYNAMIC_PERSISTENT, 0, false);
        }

        Ogre::CbDrawIndexed *drawCmd;

        if (mVaoManager->supportsIndirectBuffers())
        {
            drawCmd = reinterpret_cast<Ogre::CbDrawIndexed *>(mIndirectBuffer->map(0, mIndirectBuffer->getNumElements()));
        }
        else
        {
            drawCmd = reinterpret_cast<Ogre::CbDrawIndexed *>(mIndirectBuffer->getSwBufferPtr());
        }

        *mCommandBuffer.addCommand<Ogre::CbVao>() = Ogre::CbVao(mVao);
        *mCommandBuffer.addCommand<Ogre::CbIndirectBuffer>() = Ogre::CbIndirectBuffer(mIndirectBuffer);

        const nk_draw_command *cmd;
        unsigned int offset = 0;

		/* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &mNuklearCtx, &mCommands)
        {
            if (!cmd->elem_count) continue;

            /* glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor((GLint)(cmd->clip_rect.x * scale.x),
                (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                (GLint)(cmd->clip_rect.w * scale.x),
                (GLint)(cmd->clip_rect.h * scale.y)); */
            Ogre::CbDrawIndexed *drawCall = drawCmd++;
            drawCall->primCount = cmd->elem_count;
            drawCall->instanceCount = 1u;
            drawCall->firstVertexIndex = offset;
            drawCall->baseInstance = 0;
            drawCall->baseVertex = 0;
            offset += cmd->elem_count;
        }
        nk_clear(&mNuklearCtx);
        nk_buffer_clear(&mCommands);

        if (mVaoManager->supportsIndirectBuffers())
        {
			mIndirectBuffer->unmap(Ogre::UO_KEEP_PERSISTENT);
        }

        hlms->preCommandBufferExecution(&mCommandBuffer);
		mCommandBuffer.execute();
		hlms->postCommandBufferExecution(&mCommandBuffer);

		sceneManager->_setCurrentCompositorPass(0);

		notifyPassPosExecuteListeners();

		profilingEnd();
	}
}
