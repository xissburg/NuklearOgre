#include <OgreBuildSettings.h>
#include <GraphicsSystem.h>
#include "NuklearOgreGameState.h"

#include <OgreRoot.h>
#include <OgreWindow.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreConfigFile.h>

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
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            addResourceLocation(mResourcePath + "resources", "FileSystem", "Nuklear");
            Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Nuklear", true);

            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            mWorkspace = compositorManager->addWorkspace( mSceneManager, mRenderWindow->getTexture(),
                                                          mCamera, "NuklearWorkspace", true );
            return mWorkspace;
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load( AndroidSystems::openFile( mResourcePath + "resources2.cfg" ) );

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = AndroidSystems::isAndroid() ? "/" : "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            addResourceLocation(dataFolder + "2.0/scripts/materials/PbsMaterials", getMediaReadArchiveType(), "General");
        }

    public:
        NuklearOgreGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
            mAlwaysAskForConfig = false;
        }
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
