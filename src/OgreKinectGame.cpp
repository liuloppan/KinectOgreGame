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
#include "sdkTrays.h"

bool debugDraw = true;
//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
    : kinectController(0),
      character(0),
      dynamicsWorld(0),
      mNumofBall(0),
      mTimeSinceLastBall(0)
{
    gameTime = 1000000000; // milliseconds
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
    //std::deque<OgreBulletDynamics::RigidBody *>::iterator itBody = mBodies.begin();
    //while (mBodies.end() != itBody) {
    //    delete *itBody;
    //    ++itBody;
    //}
    //// OgreBullet physic delete - Shapes
    //std::deque<OgreBulletCollisions::CollisionShape *>::iterator itShape = mShapes.begin();
    //while (mShapes.end() != itShape) {
    //    delete *itShape;
    //    ++itShape;
    //}
    //mBodies.clear();
    //mShapes.clear();
    //delete dynamicsWorld->getDebugDrawer();
    //dynamicsWorld->setDebugDrawer(0);
    //delete dynamicsWorld;
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
    if (!BaseApplication::mousePressed(evt, id)) {
        return false;
    }

    Ogre::Real offsetX = evt.state.X.abs / float(evt.state.width);
    Ogre::Real offsetY = evt.state.Y.abs / float(evt.state.height);

    if (id == OIS::MB_Left) {
        //createBall();
    } else if (id == OIS::MB_Right) {
    }

    return true;
}
//-------------------------------------------------------------------------------------

void OgreKinectGame::setupKinect(void)
{
    kinectController = new KinectController();
    kinectController->initController();
}

void OgreKinectGame::setupWidgets()
{
    int WIDTH_UI = mWindow->getWidth() / 3;
    // main menu
    mTrayMgr->createLabel(TL_TOPRIGHT, "mTimer", "Timer: ", WIDTH_UI);
    timerLabel = (Label *) mTrayMgr->getWidget(TL_TOPRIGHT, "mTimer");

    mTrayMgr->createLabel(TL_TOPLEFT, "mScore", "Score: ", WIDTH_UI / 3);
    scoreLabel = (Label *) mTrayMgr->getWidget(TL_TOPLEFT, "mScore");

    //game over menu
    mTrayMgr->createLabel(TL_NONE, "mGameOver", "GAME OVER!", WIDTH_UI);
    mTrayMgr->createLabel(TL_NONE, "mResult", "you score is; ", WIDTH_UI);
    mTrayMgr->createButton(TL_NONE, "mQuitButton", "Quit");
    mTrayMgr->createButton(TL_NONE, "mReplayButton", "Replay");

    mTrayMgr->getWidget("mGameOver")->hide();
    mTrayMgr->getWidget("mResult")->hide();
    mTrayMgr->getWidget("mQuitButton")->hide();
    mTrayMgr->getWidget("mReplayButton")->hide();



    // mTrayMgr->showLogo(OgreBites::TL_TOP);

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::gameOver()
{
    mTrayMgr->moveWidgetToTray("mGameOver", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mResult", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mQuitButton", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mReplayButton", TL_CENTER);

    mTrayMgr->getWidget("mGameOver")->show();
    mTrayMgr->getWidget("mResult")->show();
    mTrayMgr->getWidget("mQuitButton")->show();
    mTrayMgr->getWidget("mReplayButton")->show();


    mTrayMgr->showCursor();
    mCameraMan->setStyle(CS_MANUAL);
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::buttonHit(Button *b)
{
    if (b->getName() == "mQuitButton") {
        mRoot->queueEndRendering();
    } else if (b->getName() == "mReplayButton") {

        mTrayMgr->removeWidgetFromTray("mGameOver");
        mTrayMgr->removeWidgetFromTray("mResult");
        mTrayMgr->removeWidgetFromTray("mQuitButton");
        mTrayMgr->removeWidgetFromTray("mReplayButton");

        mTrayMgr->getWidget("mGameOver")->hide();
        mTrayMgr->getWidget("mResult")->hide();
        mTrayMgr->getWidget("mQuitButton")->hide();
        mTrayMgr->getWidget("mReplayButton")->hide();

        timer->reset();
        mCameraMan->setStyle(CS_FREELOOK);
        mNumofBall = 0;

    }
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::setup(void)
{

    if (!BaseApplication::setup()) {
        return false;
    }
    // Load fonts for tray captions
    Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
    setupWidgets();

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::createScene()
{
    this->setupKinect();

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
    // Start Bullet
    dynamicsWorld = new OgreBulletDynamics::DynamicsWorld(mSceneMgr,
            Ogre::AxisAlignedBox(Ogre::Vector3(-10000, -10000, -10000),
                                 Ogre::Vector3(10000,  10000,  10000)),
            Ogre::Vector3(0, -50, 0),
            true,
            true,
            10000);

    // add Debug info display tool
    mDebugDraw = new OgreBulletCollisions::DebugDrawer();

    mDebugDraw->setDrawWireframe(true);   // we want to see the Bullet containers
    dynamicsWorld->setDebugDrawer(mDebugDraw);
    dynamicsWorld->setShowDebugShapes(true);      // enable it if you want to see the Bullet containers
    Ogre::SceneNode *debugNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("debugDrawer", Ogre::Vector3::ZERO);
    debugNode->attachObject(static_cast <Ogre::SimpleRenderable *>(mDebugDraw));


    // setup character

    character = new SinbadCharacterController();
	character->setupCharacter(this->mSceneMgr, this->kinectController, this->dynamicsWorld);

    // Floor
    Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 200000, 200000, 20, 20, true, 1, 9000, 9000, Ogre::Vector3::UNIT_Z);


    Ogre::Entity *floor = mSceneMgr->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/BeachStones");
    floor->setCastShadows(false);
    (mSceneMgr->getRootSceneNode()->createChildSceneNode("floorNode"))->attachObject(floor);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 10, 8);
	mSceneMgr->getSceneNode("floorNode")->translate(0.f,-20.f,0.f);

    OgreBulletCollisions::CollisionShape *Shape;
    Shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3(0, 1, 0), -20); // (normal vector, distance)
    OgreBulletDynamics::RigidBody *defaultPlaneBody = new OgreBulletDynamics::RigidBody(
        "BasePlane",
        dynamicsWorld);
    defaultPlaneBody->setStaticShape(Shape, 0.1, 0.8); // (shape, restitution, friction)

	// push the created objects to the deques
    mShapes.push_back(Shape);
    mBodies.push_back(defaultPlaneBody);

    //start timer
    timer = new Ogre::Timer();
}

//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameRenderingQueued(const Ogre::FrameEvent &fe)
{
    if (!BaseApplication::frameRenderingQueued(fe)) {
        return false;
    }
    mTrayMgr->frameRenderingQueued(fe);

    if ((gameTime - (long)timer->getMilliseconds()) >= 0) {
        timerString = "Timer: " + Ogre::StringConverter::toString((gameTime - timer->getMilliseconds()) / 1000);
        timerLabel->setCaption(timerString);
    } else {

        gameOver();
        return true;
    }
    kinectController->updatePerFrame(fe.timeSinceLastFrame);
    character->updatePerFrame(fe.timeSinceLastFrame);

    dynamicsWorld->stepSimulation(fe.timeSinceLastFrame);

    mTimeSinceLastBall += fe.timeSinceLastFrame;
    while (mTimeSinceLastBall >= 2.1) {
        createBall(mTimeSinceLastBall);
        mTimeSinceLastBall -= 10;
    }

    return true;
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::createBall(Ogre::Real time)
{
    if (mNumofBall == 5) {
        return;
    }
    float LO = 1;
    float HI = 50;
    float x = LO + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (HI - LO)));
    float z = LO + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (HI - LO)));

    float radius = 5;
    Ogre::Vector3 position = Ogre::Vector3(x * time, 100, z * time);
    // create an ordinary, Ogre mesh with texture
    Ogre::Entity *entity = mSceneMgr->createEntity(
                               "Ball" + Ogre::StringConverter::toString(mNumofBall),
                               "sphere.mesh");
    entity->setCastShadows(true);
    // we need the bounding box of the box to be able to set the size of the Bullet-box
    Ogre::AxisAlignedBox boundingB = entity->getBoundingBox();
    entity->setMaterialName("Bullet/Ball");
    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject(entity);
    node->scale(0.05f, 0.05f, 0.05f);
    OgreBulletCollisions::SphereCollisionShape *sceneBoxShape = new OgreBulletCollisions::SphereCollisionShape(radius);
    // and the Bullet rigid body
    OgreBulletDynamics::RigidBody *defaultBody = new OgreBulletDynamics::RigidBody(
        "defaultBoxRigid" + Ogre::StringConverter::toString(mNumofBall),
        dynamicsWorld);
    defaultBody->setShape(node,
                          sceneBoxShape,
                          5.0f,         // dynamic body restitution
                          1.0f,         // dynamic body friction
                          50.0f,          // dynamic bodymass
                          position,      // starting position of the box
                          Ogre::Quaternion(0, 0, 0, 1)); // orientation of the box
    // push the created objects to the deques
    mShapes.push_back(sceneBoxShape);
    mBodies.push_back(defaultBody);
    mNumofBall++;
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