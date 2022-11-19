#include <OgreBuildSettings.h>
#include <GraphicsSystem.h>
#include "NuklearOgreGameState.h"
#include <HlmsNuklear.h>

#include <OgreCamera.h>
#include <OgreFrustum.h>
#include <OgreRoot.h>
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
    class NuklearOgreGraphicsSystem : public GraphicsSystem
    {
        Ogre::CompositorWorkspace* setupCompositor() override
        {
            RegisterNuklearCompositor(mRoot, mRenderer);

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
                hlmsUnlit = OGRE_NEW NuklearOgre::HlmsNuklear( archiveUnlit, &archiveUnlitLibraryFolders );
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

    public:
        NuklearOgreGraphicsSystem(GameState *gameState, NuklearOgre::NuklearRenderer *renderer) :
            GraphicsSystem(gameState), mRenderer(renderer)
        {
            mAlwaysAskForConfig = false;
        }

    private:
        NuklearOgre::NuklearRenderer *mRenderer;
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        NuklearOgreGameState *gfxGameState = new NuklearOgreGameState(
            "OgreNext backend for Nuklear immediate-mode GUI. \n"
         );

        GraphicsSystem *graphicsSystem = new NuklearOgreGraphicsSystem(gfxGameState, gfxGameState);

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
