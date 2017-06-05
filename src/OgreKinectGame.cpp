/*
-----------------------------------------------------------------------------
Filename:    OgreKinectGame.cpp
-----------------------------------------------------------------------------
This source file is part of the
________                           ____  __.__                      __   
\_____  \    ___________   ____   |    |/ _|__| ____   ____   _____/  |_ 
 /   |   \  / ___\_  __ \_/ __ \  |      < |  |/    \_/ __ \_/ ___\   __\
/    |    \/ /_/  >  | \/\  ___/  |    |  \|  |   |  \  ___/\  \___|  |  
\_______  /\___  /|__|    \___  > |____|__ \__|___|  /\___  >\___  >__|  
        \//_____/             \/          \/       \/     \/     \/      
  ________                       
 /  _____/_____    _____   ____  
/   \  ___\__  \  /     \_/ __ \ 
\    \_\  \/ __ \|  Y Y  \  ___/ 
 \______  (____  /__|_|  /\___  >
        \/     \/      \/     \/ 
      Ogre Kinect Game
      https://github.com/liuloppan/KinectOgreGame/
-----------------------------------------------------------------------------
*/
#include "OgreKinectGame.h"
bool isUIvisible = false;

//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
{
    mInfo["About"] = "Ogre Kinect Game @2017.\n"
                     "Created for 3D Game Programming at Computer Scicence Yuan Ze University\n"
                     "Developer :\n"
                     "Lovisa Hassler\n"
                     "Rosdyana Kusuma\n"
                     "Project available on : https://github.com/liuloppan/KinectOgreGame/";
}
//-------------------------------------------------------------------------------------
OgreKinectGame::~OgreKinectGame()
{
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mouseMoved(const OIS::MouseEvent &evt)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectMouseMove(evt);
    }
    return BaseApplication::mouseMoved(evt);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectMouseDown(evt, id);
    }
    return BaseApplication::mousePressed(evt, id);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyPressed(const OIS::KeyEvent &evt)
{
    // relay input events to character controller
    if (!mTrayMgr->isDialogVisible()) {
        if (evt.key == OIS::KC_ESCAPE) {
            if (!isUIvisible) {
                mTrayMgr->showAll();
                setMenuVisible("MainMenu");
                setMenuVisible("Option", false);
                mTrayMgr->showCursor();
                isUIvisible = true;
            } else {
                setMenuVisible("Option", false);
                setMenuVisible("MainMenu", false);
                mTrayMgr->hideCursor();
                isUIvisible = false;
            }
        }
        if (!isUIvisible) {
            mChara->injectKeyDown(evt);
        }
    }
    return true;
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyReleased(const OIS::KeyEvent &evt)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectKeyUp(evt);
    }
    return BaseApplication::keyReleased(evt);
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::buttonHit(Button *b)
{
    if (b->getName() == "mQuitButton") {
        mRoot->queueEndRendering();
    } else if (b->getName() == "mOptionButton") {
        setMenuVisible("Option");
        setMenuVisible("MainMenu", false);
    } else if (b->getName() == "mCreditButton") {
        mTrayMgr->showOkDialog("About", mInfo["About"]);
    }
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::setup(void)
{

    if (!BaseApplication::setup()) {
        return false;
    }
    // Load fonts for tray captions
    FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
    setupWidgets();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::createScene()
{
    // setup some basic lighting for our scene
    mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
    mLight = mSceneMgr->createLight();
    mLight->setPosition(20, 20, 50);
    // disable default camera control so the character can do its own
    mCameraMan->setStyle(CS_MANUAL);
    mChara = new SinbadCharacterController(mCamera);
    // create a floor mesh resource
    MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Plane(Vector3::UNIT_Y, -18), 10000, 10000, 20, 20, true, 1, 128, 128, Vector3::UNIT_Z);


    // create a floor entity, give it a material, and place it at the origin
    mFloor = mSceneMgr->createEntity("Floor", "floor");
    mFloor->setMaterialName("Examples/BeachStones");
    mSceneMgr->getRootSceneNode()->attachObject(mFloor);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 10, 8);
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::setMenuVisible(const String &name, bool visible)
{
    if (name == "MainMenu") {
        if (visible) {
            mTrayMgr->moveWidgetToTray("mMainMenuLabel", TL_CENTER);
            mTrayMgr->moveWidgetToTray("mOptionButton", TL_CENTER);
            mTrayMgr->moveWidgetToTray("mCreditButton", TL_CENTER);
            mTrayMgr->moveWidgetToTray("mQuitButton", TL_CENTER);
            mTrayMgr->getWidget("mMainMenuLabel")->show();
            mTrayMgr->getWidget("mOptionButton")->show();
            mTrayMgr->getWidget("mCreditButton")->show();
            mTrayMgr->getWidget("mQuitButton")->show();
        } else {
            mTrayMgr->removeWidgetFromTray("mMainMenuLabel");
            mTrayMgr->removeWidgetFromTray("mOptionButton");
            mTrayMgr->removeWidgetFromTray("mCreditButton");
            mTrayMgr->removeWidgetFromTray("mQuitButton");
            mTrayMgr->getWidget("mMainMenuLabel")->hide();
            mTrayMgr->getWidget("mOptionButton")->hide();
            mTrayMgr->getWidget("mCreditButton")->hide();
            mTrayMgr->getWidget("mQuitButton")->hide();
        }
    } 
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::destroyScene()
{
    if (mChara) {
        delete mChara;
        mChara = 0;
    }
    if (mTrayMgr) {
        mTrayMgr->destroyAllWidgets();
    }
    MeshManager::getSingleton().remove("floor");

    mSceneMgr->clearScene(); // removes all nodes, billboards, lights etc.
    mSceneMgr->destroyAllCameras();
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameRenderingQueued(const FrameEvent &fe)
{
    mChara->addTime(fe.timeSinceLastFrame);
    mTrayMgr->frameRenderingQueued(fe);
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    return true;
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::setupWidgets()
{
    mTrayMgr->destroyAllWidgets();
    // create check boxes to toggle the visibility of our particle systems
    const int WIDTH_UI = 160;

    // main menu
    mTrayMgr->createLabel(TL_NONE, "mMainMenuLabel", "Main Menu", WIDTH_UI);
    mTrayMgr->createButton(TL_NONE, "mOptionButton", "Option");
    mTrayMgr->createButton(TL_NONE, "mCreditButton", "About");
    mTrayMgr->createButton(TL_NONE, "mQuitButton", "Quit");

	mTrayMgr->hideAll();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::checkBoxToggled(CheckBox *box)
{

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::itemSelected(SelectMenu *menu)
{
    Ogre::String currentElement = menu->getSelectedItem();

}
//-------------------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char *argv[])
#endif
{
    // Create application object
    OgreKinectGame app;

    try {
        app.go();
    } catch (Exception &e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
                  e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}

#ifdef __cplusplus
}
#endif