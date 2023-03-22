#include <OgreBuildSettings.h>
#include <GraphicsSystem.h>
#include "NuklearOgreGameState.h"
#include <HlmsNuklear.h>

#include <OgreCamera.h>
#include <OgreFrustum.h>
#include <OgreImage2.h>
#include <OgrePixelFormatGpu.h>
#include <OgreRoot.h>
#include <OgreTextureGpuManager.h>
#include <OgreWindow.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreConfigFile.h>
#include <OgreHlmsPbs.h>
#include <OgreArchiveManager.h>
#include <OgreHlmsManager.h>

//Declares WinMain / main
#include <MainEntryPointHelper.h>
#include <System/Android/AndroidSystems.h>
#include <System/MainEntryPoints.h>

#include "NuklearInclude.h"
#include <NuklearOgre.h>

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    return Demo::MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
}
#endif


namespace Demo
{
    void nk_font_stash_begin(nk_font_atlas *atlas)
    {
        nk_font_atlas_init_default(atlas);
        nk_font_atlas_begin(atlas);
    }

    void nk_font_stash_end(nk_font_atlas *atlas, nk_context *ctx,
                           Ogre::TextureGpuManager *textureManager,
                           nk_draw_null_texture *texNull)
    {
        const void *image; int w, h;
        image = nk_font_atlas_bake(atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

        Ogre::Image2 *imagePtr = new Ogre::Image2;
        imagePtr->loadDynamicImage(const_cast<void *>(image), w, h, 1, Ogre::TextureTypes::Type2D, Ogre::PFG_RGBA8_UNORM_SRGB, false, 1);

        Ogre::TextureGpu *texture = textureManager->createTexture("FontAtlas", Ogre::GpuPageOutStrategy::Discard,
                                                                  Ogre::TextureFlags::AutomaticBatching |
                                                                  Ogre::TextureFlags::PrefersLoadingFromFileAsSRGB,
                                                                  Ogre::TextureTypes::Type2D);
        texture->scheduleTransitionTo(Ogre::GpuResidency::Resident, imagePtr, true);

        nk_font_atlas_end(atlas, nk_handle_ptr(texture), texNull);

        if (atlas->default_font)
            nk_style_set_font(ctx, &atlas->default_font->handle);
    }

    class NuklearOgreGraphicsSystem : public GraphicsSystem
    {
        Ogre::CompositorWorkspace* setupCompositor() override
        {
            nk_init_default(&mNuklearCtx, 0);

            nk_convert_config config;
            memset(&config, 0, sizeof(config));
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = NK_ANTI_ALIASING_OFF;
            config.line_AA = NK_ANTI_ALIASING_OFF;

            /* Load Fonts: if none of these are loaded a default font will be used  */
            /* Load Cursor: if you uncomment cursor loading please hide the cursor */
            nk_font_atlas *atlas = &mFontAtlas;
            nk_font_stash_begin(atlas);
            /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
            /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
            /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
            /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
            /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
            /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
            nk_context *ctx = &mNuklearCtx;
            Ogre::TextureGpuManager *textureManager = mRoot->getHlmsManager()->getRenderSystem()->getTextureGpuManager();
            nk_font_stash_end(atlas, &mNuklearCtx, textureManager, &config.tex_null);
            /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
            /*nk_style_set_font(ctx, &roboto->handle);*/

            mNuklearRenderer.reset(new NuklearOgre::NuklearRenderer(mRoot, mSceneManager, config));
            mNuklearRenderer->addContext(&mNuklearCtx);

            NuklearOgreGameState *gameState = static_cast<NuklearOgreGameState *>(mCurrentGameState);
            gameState->mNuklearCtx = ctx;
            gameState->mNuklearRenderer = mNuklearRenderer.get();

            NuklearOgre::RegisterCustomPass(mRoot, mNuklearRenderer.get());

            addResourceLocation(mResourcePath + "resources", "FileSystem", "Nuklear");
            Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Nuklear", true);

            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            mWorkspace = compositorManager->addWorkspace( mSceneManager, mRenderWindow->getTexture(),
                                                          mCamera, "NuklearWorkspace", true );
            return mWorkspace;
        }

        void setupResources(void) override
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load( AndroidSystems::openFile( mResourcePath + "resources2.cfg" ) );

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = AndroidSystems::isAndroid() ? "/" : "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            dataFolder += "2.0/scripts/materials/PbsMaterials";

            addResourceLocation(dataFolder, getMediaReadArchiveType(), "General");
        }

    protected:
        void registerHlms(void) override
        {
            Ogre::ConfigFile cf;
            cf.load( AndroidSystems::openFile( mResourcePath + "resources2.cfg" ) );

    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            Ogre::String rootHlmsFolder = Ogre::macBundlePath() + '/' +
                                    cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
    #else
            Ogre::String rootHlmsFolder = mResourcePath + cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
    #endif

            if( rootHlmsFolder.empty() )
                rootHlmsFolder = AndroidSystems::isAndroid() ? "/" : "./";
            else if( *(rootHlmsFolder.end() - 1) != '/' )
                rootHlmsFolder += "/";

            //At this point rootHlmsFolder should be a valid path to the Hlms data folder

            NuklearOgre::HlmsNuklear *hlmsUnlit = 0;
            Ogre::HlmsPbs *hlmsPbs = 0;

            //For retrieval of the paths to the different folders needed
            Ogre::String mainFolderPath;
            Ogre::StringVector libraryFoldersPaths;
            Ogre::StringVector::const_iterator libraryFolderPathIt;
            Ogre::StringVector::const_iterator libraryFolderPathEn;

            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

            const Ogre::String &archiveType = getMediaReadArchiveType();

            {
                //Create & Register HlmsNuklear
                //Get the path to all the subdirectories used by HlmsNuklear
                NuklearOgre::HlmsNuklear::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Ogre::Archive *archiveUnlit = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                                archiveType, true );
                Ogre::ArchiveVec archiveUnlitLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Ogre::Archive *archiveLibrary =
                            archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, archiveType, true );
                    archiveUnlitLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }

                //Create and register the unlit Hlms
                Ogre::Real windowScale = 2;
                hlmsUnlit = OGRE_NEW NuklearOgre::HlmsNuklear( archiveUnlit, &archiveUnlitLibraryFolders, windowScale );
                Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );

                // Load custom Nuklear shader pieces.
                Ogre::ArchiveVec libraryUnlit = hlmsUnlit->getPiecesLibraryAsArchiveVec();
                libraryUnlit.push_back( Ogre::ArchiveManager::getSingletonPtr()->load(
                                        mResourcePath + "resources/Hlms/",
                                        getMediaReadArchiveType(), true ) );
                hlmsUnlit->reloadFrom( archiveUnlit, &libraryUnlit );
            }

            {
                //Create & Register HlmsPbs
                //Do the same for HlmsPbs:
                Ogre::HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
                Ogre::Archive *archivePbs = archiveManager.load( rootHlmsFolder + mainFolderPath,
                                                                archiveType, true );

                //Get the library archive(s)
                Ogre::ArchiveVec archivePbsLibraryFolders;
                libraryFolderPathIt = libraryFoldersPaths.begin();
                libraryFolderPathEn = libraryFoldersPaths.end();
                while( libraryFolderPathIt != libraryFolderPathEn )
                {
                    Ogre::Archive *archiveLibrary =
                            archiveManager.load( rootHlmsFolder + *libraryFolderPathIt, archiveType, true );
                    archivePbsLibraryFolders.push_back( archiveLibrary );
                    ++libraryFolderPathIt;
                }

                //Create and register
                hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &archivePbsLibraryFolders );
                Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
            }


            Ogre::RenderSystem *renderSystem = mRoot->getRenderSystem();
            if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" )
            {
                //Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
                //and below to avoid saturating AMD's discard limit (8MB) or
                //saturate the PCIE bus in some low end machines.
                bool supportsNoOverwriteOnTextureBuffers;
                renderSystem->getCustomAttribute( "MapNoOverwriteOnDynamicBufferSRV",
                                                &supportsNoOverwriteOnTextureBuffers );

                if( !supportsNoOverwriteOnTextureBuffers )
                {
                    hlmsPbs->setTextureBufferDefaultSize( 512 * 1024 );
                    hlmsUnlit->setTextureBufferDefaultSize( 512 * 1024 );
                }
            }
        }

        void deinitialize() override
        {
            nk_font_atlas_clear(&mFontAtlas);
            nk_free(&mNuklearCtx);
            mNuklearRenderer.reset();

            GraphicsSystem::deinitialize();
        }

    public:
        NuklearOgreGraphicsSystem(GameState *gameState) :
            GraphicsSystem(gameState)
        {
            mAlwaysAskForConfig = true;
        }

    private:
        nk_context mNuklearCtx;
        nk_font_atlas mFontAtlas;
        std::unique_ptr<NuklearOgre::NuklearRenderer> mNuklearRenderer;
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        NuklearOgreGameState *gfxGameState = new NuklearOgreGameState(
            "OgreNext backend for Nuklear immediate-mode GUI. \n"
         );

        GraphicsSystem *graphicsSystem = new NuklearOgreGraphicsSystem(gfxGameState);

        gfxGameState->_notifyGraphicsSystem( graphicsSystem );

        *outGraphicsGameState = gfxGameState;
        *outGraphicsSystem = graphicsSystem;
    }

    void MainEntryPoints::destroySystems( GameState *graphicsGameState,
                                          GraphicsSystem *graphicsSystem,
                                          GameState *logicGameState,
                                          LogicSystem *logicSystem )
    {
        delete graphicsSystem;
        delete graphicsGameState;
    }

    const char* MainEntryPoints::getWindowTitle(void)
    {
        return "NuklearOgre";
    }
}
