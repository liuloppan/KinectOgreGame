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
#include "Stdafx.h"
#include "OgreKinectGame.h"

bool isUIvisible = false;
#include "sdkTrays.h"
//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
    : kinectController(0),
      character(0),
      accumulator(0),
      dt(0.01),
      dynamicsWorld(0),
      ogreDisplay(0)
{
	gameTime = 5000; // milliseconds
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
	    if (mTrayMgr) {
        mTrayMgr->destroyAllWidgets();
    }
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::destroyScene()
{
    if (kinectController) {
        kinectController->unInitController();
        delete kinectController;
    }

    if (character) {
        delete character;
    }

    if (dynamicsWorld) {
        delete dynamicsWorld;
    }

    if (ogreDisplay) {
        delete ogreDisplay;
    }

    Ogre::MeshManager::getSingleton().remove("floor");

    mSceneMgr->clearScene(); // removes all nodes, billboards, lights etc.
    mSceneMgr->destroyAllCameras();
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyReleased(const OIS::KeyEvent &evt)
{
    return BaseApplication::keyReleased(evt);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mouseMoved(const OIS::MouseEvent &evt)
{
    return BaseApplication::mouseMoved(evt);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
    return BaseApplication::mousePressed(evt, id);
}
//-------------------------------------------------------------------------------------

void OgreKinectGame::setupKinect(void)
{
    kinectController = new KinectController();
    kinectController->initController();

}

void OgreKinectGame::setupWidgets()
{
	int WIDTH_UI = mWindow->getWidth()/3;
	// main menu
    mTrayMgr->createLabel(TL_TOPRIGHT, "mTimer", "Timer: ", WIDTH_UI);
    timerLabel = (Label*) mTrayMgr->getWidget(TL_TOPRIGHT,"mTimer");

	mTrayMgr->createLabel(TL_TOPLEFT, "mScore", "Score: ", WIDTH_UI/3);
	scoreLabel = (Label*) mTrayMgr->getWidget(TL_TOPLEFT,"mScore");

	//game over menu
	mTrayMgr->createLabel(TL_NONE, "mGameOver", "GAME OVER!", WIDTH_UI);
	mTrayMgr->createLabel(TL_NONE, "mResult", "you score is; ", WIDTH_UI);
	mTrayMgr->createButton(TL_NONE, "mQuitButton", "Quit");

	mTrayMgr->getWidget("mGameOver")->hide();
    mTrayMgr->getWidget("mResult")->hide();
	mTrayMgr->getWidget("mQuitButton")->hide();




   // mTrayMgr->showLogo(OgreBites::TL_TOP);

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::gameOver()
{
	mTrayMgr->moveWidgetToTray("mGameOver", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mResult", TL_CENTER);
	mTrayMgr->moveWidgetToTray("mQuitButton", TL_CENTER);

	mTrayMgr->getWidget("mGameOver")->show();
    mTrayMgr->getWidget("mResult")->show();
	mTrayMgr->getWidget("mQuitButton")->show();

	mTrayMgr->showCursor();

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
    this->setupKinect();

    // setup character
    character = new SinbadCharacterController();
    character->setupCharacter(this->mSceneMgr, this->kinectController);
    //character->getEntityNode()->rotate(Ogre::Vector3(0,1,0), Ogre::Degree(180));

    // setup shadow properties
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
    mSceneMgr->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));

    // setup some basic lighting for our scene
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.8, 0.8, 0.8));

    // add a bright light above the scene
    mLight = mSceneMgr->createLight();
    mLight->setType(Ogre::Light::LT_POINT);
    mLight->setPosition(-100, 800, 400);
    mLight->setSpecularColour(Ogre::ColourValue::White);

    // Bullet Physics

    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));


    //create the character physical skeleton
    ogreDisplay = new OgreDisplay(dynamicsWorld);
    ragdoll = new SkeletonToRagdoll(mSceneMgr);
    ragdoll->createRagdoll(dynamicsWorld, character->getEntityNode());
    ragdoll->setDebugBones(false);

    // Floor
    Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 1000, 1000, 10, 10, true, 1, 10, 10, Ogre::Vector3::UNIT_Z);


    Ogre::Entity *floor = mSceneMgr->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/BeachStones");
    floor->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->attachObject(floor);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 10, 8);

    // create the Bullet ground plane

    btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    //btCollisionShape *groundShape = new btBoxShape(btVector3(350, 1, 350));
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, 0, 0));
    btDefaultMotionState *myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, myMotionState, groundShape, btVector3(0, 0, 0));
    groundRigidBodyCI.m_additionalDamping = true;
    btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setFriction(50.0f);
    dynamicsWorld->addRigidBody(groundRigidBody);
    ragdoll->addIgnoreEventObject(groundRigidBody);

	//start timer
	timer = new Ogre::Timer();

    // Color Data
    //texRenderTarget = Ogre::TextureManager::getSingleton().createManual("texRenderTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    //                  Ogre::TEX_TYPE_2D, 320, 240, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);

    //Ogre::Rectangle2D *mMiniScreen = new Ogre::Rectangle2D(true);
    //mMiniScreen->setCorners(0.5f, -0.5f, 1.0f, -1.0f);
    //mMiniScreen->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));

    //Ogre::SceneNode *miniScreenNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MiniScreenNode");
    //miniScreenNode->attachObject(mMiniScreen);
}

//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameRenderingQueued(const Ogre::FrameEvent &fe)
{


	mTrayMgr->frameRenderingQueued(fe);
	
	if((gameTime - (long)timer->getMilliseconds()) >= 0)
	{
		timerString = "Timer: " + Ogre::StringConverter::toString((gameTime - timer->getMilliseconds())/1000);
		timerLabel->setCaption(timerString);
	}else{
	
		gameOver();
		return true;
	}

    kinectController->updatePerFrame(fe.timeSinceLastFrame);
    character->updatePerFrame(fe.timeSinceLastFrame);

    // Update Color Data
    this->kinectController->showColorData(this->texRenderTarget);
    if (dynamicsWorld) {
        accumulator += fe.timeSinceLastFrame;
        if (accumulator >= dt) {
            dynamicsWorld->stepSimulation(dt);
            accumulator -= dt;
            //get all colliding objects and check for specific collisions
            dynamicsWorld->performDiscreteCollisionDetection();
            ogreDisplay->update();

            ragdoll->update();
            //std::string name = ragdoll->update();
            //if(name != "") name = name;
        }
    }

    return true;
}
//-------------------------------------------------------------------------------------


#include "Stdafx.h"
#include "OgreKinectGame.h"


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
    } catch (Ogre::Exception &e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
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