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
// to checking memory leak
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#include "Stdafx.h"
#include "OgreKinectGame.h"
#include "sdkTrays.h"

bool debugDraw = true;
bool isUIvisible = false;
//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
    : kinectController(0),
      character(0),
      dynamicsWorld(0),
      mNumofBall(0),
      mTimeSinceLastBall(0),
      mBallEntity(0),
      score(0),
      gameTime(50000),
      particleCounter(0),
      particleLifeTime(250)
      //mElementNode(0)
{
    // to checking memory leak
    //_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
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
    // to checking memory leak
    //_CrtDumpMemoryLeaks();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::destroyScene()
{
    if (mTrayMgr) {
        delete mTrayMgr;
    }
    if (kinectController) {
        kinectController->unInitController();
        kinectController = NULL;
        delete kinectController;
    }

    if (character) {
        character = NULL;
        delete character;
    }

    if (dynamicsWorld) {
        dynamicsWorld = NULL;
        delete dynamicsWorld;
    }

    if (mDebugDraw) {
        mDebugDraw = NULL;
        delete mDebugDraw;
    }

    if (Shape) {
        Shape = NULL;
        delete Shape;
    }

    if (defaultPlaneBody) {
        defaultPlaneBody = NULL;
        delete defaultPlaneBody;
    }

    if (BallShape) {
        BallShape = NULL;
        delete BallShape;
    }

    if (rbBall) {
        rbBall = NULL;
        delete rbBall;
    }

    Ogre::MeshManager::getSingleton().remove("floor");
    if (!mSceneMgr) {
        mSceneMgr->clearScene(); // removes all nodes, billboards, lights etc.
        mSceneMgr->destroyAllCameras();
    }

}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyPressed(const OIS::KeyEvent &evt)
{
    switch (evt.key) {
        case OIS::KC_ESCAPE:
            if (!isUIvisible) {
                gamePause();
                isUIvisible = true;
            } else {
                returnGame();
                isUIvisible = false;
            }
            break;
        default:
            break;
    }

    return true;
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

    scoreString = "Your final score is " + Ogre::StringConverter::toString(score) + "!";
    mTrayMgr->createLabel(TL_NONE, "mResult", scoreString, WIDTH_UI);
    mTrayMgr->getWidget("mResult")->hide();
    //game over menu
    mTrayMgr->createLabel(TL_NONE, "mGameOver", "GAME OVER!", WIDTH_UI);
    mTrayMgr->createButton(TL_NONE, "mQuitButton", "Quit");
    mTrayMgr->createButton(TL_NONE, "mReplayButton", "Replay");

    mTrayMgr->getWidget("mGameOver")->hide();
    mTrayMgr->getWidget("mQuitButton")->hide();
    mTrayMgr->getWidget("mReplayButton")->hide();

    // game pause
    mTrayMgr->createLabel(TL_NONE, "mGameMenu", "MAIN MENU", WIDTH_UI);
    mTrayMgr->getWidget("mGameMenu")->hide();

    // mTrayMgr->showLogo(OgreBites::TL_TOP);

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::gameOver()
{
    mTrayMgr->moveWidgetToTray("mGameOver", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mResult", TL_CENTER);
    scoreString = "Your final score is " + Ogre::StringConverter::toString(score) + "!";
    ((OgreBites::Label *)mTrayMgr->getWidget("mResult"))->setCaption(scoreString);
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
void OgreKinectGame::gamePause()
{
    mTrayMgr->moveWidgetToTray("mGameMenu", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mQuitButton", TL_CENTER);
    mTrayMgr->moveWidgetToTray("mReplayButton", TL_CENTER);

    mTrayMgr->getWidget("mGameMenu")->show();
    mTrayMgr->getWidget("mQuitButton")->show();
    mTrayMgr->getWidget("mReplayButton")->show();

    mTrayMgr->showCursor();
    mCameraMan->setStyle(CS_MANUAL);
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::returnGame()
{
    mTrayMgr->removeWidgetFromTray("mGameMenu");
    mTrayMgr->removeWidgetFromTray("mQuitButton");
    mTrayMgr->removeWidgetFromTray("mReplayButton");

    mTrayMgr->getWidget("mGameMenu")->hide();
    mTrayMgr->getWidget("mQuitButton")->hide();
    mTrayMgr->getWidget("mReplayButton")->hide();

    mTrayMgr->showCursor();
    mCameraMan->setStyle(CS_MANUAL);
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::addScorePoint(int point)
{
    score += point;

    scoreString = "Score: " + Ogre::StringConverter::toString(score);

    scoreLabel->setCaption(scoreString);
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
        mTrayMgr->removeWidgetFromTray("mGameMenu");
        mTrayMgr->getWidget("mGameMenu")->hide();
        mTrayMgr->getWidget("mGameOver")->hide();
        mTrayMgr->getWidget("mResult")->hide();
        mTrayMgr->getWidget("mQuitButton")->hide();
        mTrayMgr->getWidget("mReplayButton")->hide();

        timer->reset();
        mCameraMan->setStyle(CS_FREELOOK);
        scoreLabel->setCaption("Score: 0");
        score = 0;
        mTrayMgr->hideCursor();
        //handle the particlesystems
        for (int i = particleCounter; i > 0; i--) {
            if (!mParticleSystems.empty()) {
                //delete mParticleSystems.at(i);
                mParticleSystems.pop_back();
                mSceneMgr->destroySceneNode("particleSysNode" + Ogre::StringConverter::toString(i));
                mSceneMgr->destroyParticleSystem("Fire" + Ogre::StringConverter::toString(i));
            }
        }
        particleCounter = 0;

        // ball
        mBallEntity->_deinitialise();
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
    mSceneMgr->getSceneNode("floorNode")->translate(0.f, -20.f, 0.f);

    Shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3(0, 1, 0), -20); // (normal vector, distance)
    defaultPlaneBody = new OgreBulletDynamics::RigidBody(
        "BasePlane",
        dynamicsWorld);
    defaultPlaneBody->setStaticShape(Shape, 0.1, 0.8); // (shape, restitution, friction)

    // push the created objects to the deques
    mShapes.push_back(Shape);
    mBodies.push_back(defaultPlaneBody);

    //start timer
    timer = new Ogre::Timer();

    // particle
    //    setupParticle();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::checkCollisions()
{
    btCollisionWorld *collisionWorld = dynamicsWorld->getBulletCollisionWorld();
    btDynamicsWorld *dynamicWorld = dynamicsWorld->getBulletDynamicsWorld();

    int numManifolds = collisionWorld->getDispatcher()->getNumManifolds();
    bool collide = false;
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold *contactManifold =  collisionWorld->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject *obA = (btCollisionObject *) contactManifold->getBody0();
        btCollisionObject *obB = (btCollisionObject *) contactManifold->getBody1();

        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++) {
            btManifoldPoint &pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.f) {
                const btVector3 &ptA = pt.getPositionWorldOnA();
                const btVector3 &ptB = pt.getPositionWorldOnB();
                const btVector3 &normalOnB = pt.m_normalWorldOnB;
                collide = true;
                const char *nameA = obA-> getCollisionShape()->getName();
                const char *nameB = obB->getCollisionShape()->getName();
                std::cout << "Collision Body A: " <<  nameA << std::endl;
                std::cout << "Collision Body B: " << nameB << std::endl;

                if (obA->getUserPointer() == "player") {
                    addScorePoint(1);

                    std::pair<Ogre::ParticleSystem *, int> particlePair;
                    int lifeTime = particleLifeTime;
                    Ogre::ParticleSystem *particleSys = createParticle(OgreBulletCollisions::BtOgreConverter::to(ptB));
                    particlePair.first = particleSys;
                    particlePair.second = lifeTime;
                    mParticleSystems.push_back(particlePair);


                } else if (obB->getUserPointer() == "player"){
                    addScorePoint(1);
                    std::pair<Ogre::ParticleSystem *, int> particlePair;
                    int lifeTime = particleLifeTime;
                    Ogre::ParticleSystem *particleSys = createParticle(OgreBulletCollisions::BtOgreConverter::to(ptA));
                    particlePair.first = particleSys;
                    particlePair.second = lifeTime;
                    mParticleSystems.push_back(particlePair);
                }
            }
        }
    }
}
//-------------------------------------------------------------------------------------
Ogre::ParticleSystem *OgreKinectGame::createParticle(Ogre::Vector3 &pos)
{
    particleCounter++;
    Ogre::ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);

    Ogre::SceneNode *mElementNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("particleSysNode" + Ogre::StringConverter::toString(particleCounter), pos);
    Ogre::ParticleSystem *mParticleSys = mSceneMgr->createParticleSystem("Fire" + Ogre::StringConverter::toString(particleCounter), "Elements/Fire");

    mElementNode->attachObject(mParticleSys);
    mParticleSys->setVisible(true);

    return mParticleSys;
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


    for (int i = 0 ; i < mParticleSystems.size(); i++) {
        int particleLifeTime = mParticleSystems.at(i).second;
        if (particleLifeTime > 0) {
            mParticleSystems.at(i).first->setEmitting(true);
        } else {
            mParticleSystems.at(i).first->setEmitting(false);
        }

        particleLifeTime--;
        mParticleSystems.at(i).second = particleLifeTime;

    }

    kinectController->updatePerFrame(fe.timeSinceLastFrame);
    character->updatePerFrame(fe.timeSinceLastFrame);

    dynamicsWorld->stepSimulation(fe.timeSinceLastFrame);

    mTimeSinceLastBall += fe.timeSinceLastFrame;
    while (mTimeSinceLastBall >= 2.1) {
        createBall(mTimeSinceLastBall);
        mTimeSinceLastBall -= 5;
    }
    checkCollisions();
    return true;
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::createBall(Ogre::Real time)
{

    float LO = -5;
    float HI = 10;
    float x = LO + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (HI - LO)));
    float z = LO + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (HI - LO)));

    float radius = 5;
    Ogre::Vector3 position = character->getEntityNode()->getPosition(); // Ogre::Vector3(x * time, 100, z * time);
    position.y += 50.f;
	// create an ordinary, Ogre mesh with texture
    mBallEntity = mSceneMgr->createEntity(
                      "Ball" + Ogre::StringConverter::toString(mNumofBall),
                      "sphere.mesh");
    mBallEntity->setCastShadows(true);
    // we need the bounding box of the box to be able to set the size of the Bullet-box
    Ogre::AxisAlignedBox boundingB = mBallEntity->getBoundingBox();
    mBallEntity->setMaterialName("Bullet/Ball");
    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject(mBallEntity);
    node->scale(0.05f, 0.05f, 0.05f);
    BallShape = new OgreBulletCollisions::SphereCollisionShape(radius);
    // and the Bullet rigid body
    rbBall = new OgreBulletDynamics::RigidBody(
        "defaultBoxRigid" + Ogre::StringConverter::toString(mNumofBall),
        dynamicsWorld);
    rbBall->setShape(node,
                     BallShape,
                     10.0f,         // dynamic body restitution
                     1.0f,         // dynamic body friction
                     50.0f,          // dynamic bodymass
                     position,      // starting position of the box
                     Ogre::Quaternion(0, 0, 0, 1)); // orientation of the box
    // push the created objects to the deques
    mShapes.push_back(BallShape);
    mBodies.push_back(rbBall);
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