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

//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
    : kinectController(0),
      character(0),
      accumulator(0),
      dt(0.01),
      dynamicsWorld(0),
      ogreDisplay(0)
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
    if (kinectController) {
        kinectController->unInitController();
        delete kinectController;
    }

    if (character) {
        delete character;
    }

    Ogre::MeshManager::getSingleton().remove("floor");

    mSceneMgr->clearScene(); // removes all nodes, billboards, lights etc.
    mSceneMgr->destroyAllCameras();
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

//-------------------------------------------------------------------------------------
void OgreKinectGame::createScene()
{
    this->setupKinect();

    // setup character
    character = new SinbadCharacterController();
    character->setupCharacter(this->mSceneMgr, this->kinectController);

    // setup shadow properties
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
    mSceneMgr->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    mSceneMgr->setShadowTextureSize(2048);
    mSceneMgr->setShadowTextureCount(1);

    // setup some basic lighting for our scene
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));

    // add a bright light above the scene
    mLight = mSceneMgr->createLight();
    mLight->setType(Ogre::Light::LT_POINT);
    mLight->setPosition(-100, 400, 200);
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
}

//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameRenderingQueued(const Ogre::FrameEvent &fe)
{
    if (!BaseApplication::frameRenderingQueued(fe)) {
        return false;
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