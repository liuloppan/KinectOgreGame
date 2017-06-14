/*
-----------------------------------------------------------------------------
Filename:    BaseApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "Stdafx.h"
#include "BaseApplication.h"
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;

size_t BaseApplication::mNumEntitiesInstanced = 0;
const float      gStaticBodyRestitution  = 0.1f;
const float      gStaticBodyFriction     = 0.8f;
const Ogre::Vector3    gCubeBodyBounds      = Ogre::Vector3 (1, 1, 1);
const Ogre::Vector3    gCylinderBodyBounds  = Ogre::Vector3 (0.5, 1, 0.5); 
const Ogre::Vector3    gConeBodyBounds      = Ogre::Vector3 (1, 1, 1);
const float      gDynamicBodyRestitution = 0.6f;
const float      gDynamicBodyFriction    = 0.6f;
const float      gDynamicBodyMass        = 1.0f;
const Ogre::Real       gSphereBodyBounds    = 1.0f;
//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
    : mRoot(0),
      mCamera(0),
      mSceneMgr(0),
      mWindow(0),
      mResourcesCfg(Ogre::StringUtil::BLANK),
      mPluginsCfg(Ogre::StringUtil::BLANK),
      mTrayMgr(0),
      mCameraMan(0),
      mDetailsPanel(0),
      mCursorWasVisible(false),
      mShutDown(false),
      mInputManager(0),
      mMouse(0),
      mKeyboard(0),
	  mWorld(0)
{
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    if (mTrayMgr) {
        delete mTrayMgr;
    }
    if (mCameraMan) {
        delete mCameraMan;
    }

    delete mWorld->getDebugDrawer();
    mWorld->setDebugDrawer(0);
    delete mWorld;
	mWorld = 0;
    mBodies.clear();
    mEntities.clear();

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if (mRoot->showConfigDialog()) {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Ogre Kinect Game");

        return true;
    } else {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0, 80, 300));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0, 40, 0));
    mCamera->setNearClipDistance(0.5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem(pl);

    mKeyboard = static_cast<OIS::Keyboard *>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mMouse = static_cast<OIS::Mouse *>(mInputManager->createInputObject(OIS::OISMouse, true));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport *vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup()) {
        return;
    }

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) {
        return false;
    }

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
    if (mWindow->isClosed()) {
        return false;
    }

    if (mShutDown) {
        return false;
    }

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible()) {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible()) { // if details panel is visible, then update its contents
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }

    return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::keyPressed(const OIS::KeyEvent &arg)
{
    if (mTrayMgr->isDialogVisible()) {
        return true;    // don't process any more keys if dialog is up
    }

    if (arg.key == OIS::KC_F) { // toggle visibility of advanced frame stats
        mTrayMgr->toggleAdvancedFrameStats();
    } else if (arg.key == OIS::KC_G) { // toggle visibility of even rarer debugging details
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE) {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        } else {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    } else if (arg.key == OIS::KC_T) { // cycle polygon rendering mode
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0]) {
            case 'B':
                newVal = "Trilinear";
                tfo = Ogre::TFO_TRILINEAR;
                aniso = 1;
                break;
            case 'T':
                newVal = "Anisotropic";
                tfo = Ogre::TFO_ANISOTROPIC;
                aniso = 8;
                break;
            case 'A':
                newVal = "None";
                tfo = Ogre::TFO_NONE;
                aniso = 1;
                break;
            default:
                newVal = "Bilinear";
                tfo = Ogre::TFO_BILINEAR;
                aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    } else if (arg.key == OIS::KC_R) { // cycle polygon rendering mode
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode()) {
            case Ogre::PM_SOLID:
                newVal = "Wireframe";
                pm = Ogre::PM_WIREFRAME;
                break;
            case Ogre::PM_WIREFRAME:
                newVal = "Points";
                pm = Ogre::PM_POINTS;
                break;
            default:
                newVal = "Solid";
                pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    } else if (arg.key == OIS::KC_F5) { // refresh all textures
        Ogre::TextureManager::getSingleton().reloadAll();
    } else if (arg.key == OIS::KC_SYSRQ) { // take a screenshot
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    } else if (arg.key == OIS::KC_ESCAPE) {
        mShutDown = true;
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}

bool BaseApplication::keyReleased(const OIS::KeyEvent &arg)
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool BaseApplication::mouseMoved(const OIS::MouseEvent &arg)
{
    if (mTrayMgr->injectMouseMove(arg)) {
        return true;
    }
    //mCameraMan->injectMouseMove(arg);
    return true;
}

bool BaseApplication::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseDown(arg, id)) {
        return true;
    }
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool BaseApplication::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseUp(arg, id)) {
        return true;
    }
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow *rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow *rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if (rw == mWindow) {
        if (mInputManager) {
            mInputManager->destroyInputObject(mMouse);
            mInputManager->destroyInputObject(mKeyboard);

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

// bullet
// -------------------------------------------------------------------------
void BaseApplication::initWorld(const Ogre::Vector3 &gravityVector, 
                                   const Ogre::AxisAlignedBox &bounds)
{
    // Start Bullet
    mWorld = new DynamicsWorld (mSceneMgr, bounds, gravityVector, true, true, 10000);

    // add Debug info display tool
    DebugDrawer *debugDrawer = new DebugDrawer();

    mWorld->setDebugDrawer(debugDrawer);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("debugDrawer", Ogre::Vector3::ZERO);
    node->attachObject (static_cast <Ogre::SimpleRenderable *> (debugDrawer));


}
// -------------------------------------------------------------------------
void BaseApplication::addGround()
{
    addStaticPlane(
        gStaticBodyRestitution, 
        gStaticBodyFriction);
}
// -------------------------------------------------------------------------
RigidBody *BaseApplication::addCube(const Ogre::String instanceName,
                                       const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
                                       const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
                                       const Ogre::Real bodyMass)
{
    Ogre::Entity *entity = mSceneMgr->createEntity(
        instanceName + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        "Bulletbox.mesh");
    // "Crate.mesh");
    // "Crate1.mesh");
    // "Crate2.mesh");


    entity->setQueryFlags (GEOMETRY_QUERY_MASK);
#if (OGRE_VERSION < ((1 << 16) | (5 << 8) | 0)) // only applicable before shoggoth (1.5.0)
    entity->setNormaliseNormals(true);
#endif
	entity->setCastShadows(true);

    entity->setMaterialName("Bullet/box");

    BoxCollisionShape *sceneCubeShape = new BoxCollisionShape(size);

    RigidBody *defaultBody = new RigidBody(
        "defaultCubeRigid" + Ogre::StringConverter::toString(mNumEntitiesInstanced), 
        mWorld);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
    node->attachObject (entity);

    defaultBody->setShape (node,  sceneCubeShape, bodyRestitution, bodyFriction, bodyMass, pos, q);

    mEntities.push_back(entity);
    mShapes.push_back(sceneCubeShape);
    mBodies.push_back(defaultBody);
    mNumEntitiesInstanced++;

    return defaultBody;
}
// -------------------------------------------------------------------------
RigidBody *BaseApplication::addSphere(const Ogre::String instanceName,
                                         const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Real radius,
                                         const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
                                         const Ogre::Real bodyMass)
{
    Ogre::Entity *entity = mSceneMgr->createEntity(
        instanceName + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        "ellipsoid.mesh");

    entity->setQueryFlags (GEOMETRY_QUERY_MASK);
#if (OGRE_VERSION < ((1 << 16) | (5 << 8) | 0)) // only applicable before shoggoth (1.5.0)
    entity->setNormaliseNormals(true);
#endif
	entity->setCastShadows(true);

    entity->setMaterialName("Bullet/box");

    SphereCollisionShape *sceneCubeShape = new SphereCollisionShape(radius);

    RigidBody *defaultBody = new RigidBody(
        "defaultSphereRigid" + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        mWorld);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
    node->attachObject (entity);

    defaultBody->setShape (node,  sceneCubeShape, bodyRestitution, bodyFriction, bodyMass, pos, q);

    mEntities.push_back(entity);
    mShapes.push_back(sceneCubeShape);
    mBodies.push_back(defaultBody);
    mNumEntitiesInstanced++;

    return defaultBody;
}

// -------------------------------------------------------------------------
RigidBody *BaseApplication::addCylinder(const Ogre::String instanceName,
                                           const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
                                           const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
                                           const Ogre::Real bodyMass)
{
    Ogre::Entity *entity = mSceneMgr->createEntity(
        instanceName + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        "cylinder.mesh");
    //   "capsule.mesh");
    // "Barrel.mesh");
    // "Barrel1.mesh");
    // "Barrel2.mesh");

    entity->setQueryFlags (GEOMETRY_QUERY_MASK);
#if (OGRE_VERSION < ((1 << 16) | (5 << 8) | 0)) // only applicable before shoggoth (1.5.0)
    entity->setNormaliseNormals(true);
#endif
	entity->setCastShadows(true);

    entity->setMaterialName("Bullet/box");

    CylinderCollisionShape *sceneCubeShape = new CylinderCollisionShape(size, Ogre::Vector3::UNIT_X);

    RigidBody *defaultBody = new RigidBody(
        "defaultCylinderRigid" + Ogre::StringConverter::toString(mNumEntitiesInstanced), 
        mWorld);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
    node->attachObject (entity);

    defaultBody->setShape (node, sceneCubeShape, bodyRestitution, bodyFriction, bodyMass, pos, q);

    mEntities.push_back(entity);
    mShapes.push_back(sceneCubeShape);
    mBodies.push_back(defaultBody);
    mNumEntitiesInstanced++;

    return defaultBody;
}
// -------------------------------------------------------------------------
RigidBody *BaseApplication::addCone(const Ogre::String instanceName,
                                       const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
                                       const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
                                       const Ogre::Real bodyMass)
{
    Ogre::Entity *entity = mSceneMgr->createEntity(
        instanceName + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        //"Cone.mesh");
        "Mesh.mesh");

    entity->setQueryFlags (GEOMETRY_QUERY_MASK);
#if (OGRE_VERSION < ((1 << 16) | (5 << 8) | 0)) // only applicable before shoggoth (1.5.0)
    entity->setNormaliseNormals(true);
#endif
	entity->setCastShadows(true);

    entity->setMaterialName("Bullet/box");

    ConeCollisionShape *sceneCubeShape = new ConeCollisionShape(size.x, size.y, Ogre::Vector3::UNIT_Y);

    RigidBody *defaultBody = new RigidBody(
        "defaultConeRigid" + Ogre::StringConverter::toString(mNumEntitiesInstanced), 
        mWorld);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
    node->attachObject (entity);

    defaultBody->setShape (node, sceneCubeShape, bodyRestitution, bodyFriction, bodyMass, pos, q);

    mEntities.push_back(entity);
    mShapes.push_back(sceneCubeShape);
    mBodies.push_back(defaultBody);
    mNumEntitiesInstanced++;

    return defaultBody;
}
// -------------------------------------------------------------------------
RigidBody *BaseApplication::addStaticTrimesh(const Ogre::String &instanceName,
                                                const Ogre::String &meshName,
                                                const Ogre::Vector3 &pos, 
                                                const Ogre::Quaternion &q, 
                                                const Ogre::Real bodyRestitution, 
                                                const Ogre::Real bodyFriction,
                                                bool castShadow)
{
    Ogre::Entity *sceneEntity = mSceneMgr->createEntity(instanceName + Ogre::StringConverter::toString(mNumEntitiesInstanced), meshName);
    sceneEntity->setCastShadows (castShadow);

    StaticMeshToShapeConverter *trimeshConverter = new StaticMeshToShapeConverter(sceneEntity);
    TriangleMeshCollisionShape *sceneTriMeshShape = trimeshConverter->createTrimesh();
    delete trimeshConverter;
    RigidBody *sceneRigid = new RigidBody(
        instanceName + "Rigid" + Ogre::StringConverter::toString(mNumEntitiesInstanced),
        mWorld);

    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode ()->createChildSceneNode ();
    node->attachObject (sceneEntity);

    sceneRigid->setStaticShape(node, sceneTriMeshShape, bodyRestitution, bodyFriction, pos);

    mEntities.push_back(sceneEntity);
    mBodies.push_back(sceneRigid);
    mNumEntitiesInstanced++;

    return sceneRigid;
}
// -------------------------------------------------------------------------
RigidBody *BaseApplication::addStaticPlane( const Ogre::Real bodyRestitution, 
                                              const Ogre::Real bodyFriction)
{
    // Use a load of meshes to represent the floor
    int i = 0;
    Ogre::StaticGeometry* s;
    s = mSceneMgr->createStaticGeometry("StaticFloor");
    s->setRegionDimensions(Ogre::Vector3(160.0, 100.0, 160.0));
    // Set the region origin so the center is at 0 world
    s->setOrigin(Ogre::Vector3::ZERO);
    for (Ogre::Real z = -80.0;z <= 80.0;z += 20.0)
    {
        for (Ogre::Real x = -80.0;x <= 80.0;x += 20.0)
        {
            Ogre::String name = Ogre::String("Plane_") + Ogre::StringConverter::toString(i++);

            Ogre::Entity* entity = mSceneMgr->createEntity(name, "plane.mesh");
			entity->setMaterialName("BulletPlane");
            entity->setQueryFlags (STATIC_GEOMETRY_QUERY_MASK);
            //entity->setUserObject(_plane);
            entity->setCastShadows(false);
            s->addEntity(entity, Ogre::Vector3(x,0,z));
        }
    }
    s->build();
    //SceneNode* mPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);


    CollisionShape *Shape = new StaticPlaneCollisionShape (Ogre::Vector3(0,1,0), 0);

    RigidBody *defaultPlaneBody = new RigidBody(
        "Plane" + Ogre::StringConverter::toString(mNumEntitiesInstanced), 
        mWorld);


    defaultPlaneBody->setStaticShape (Shape, bodyRestitution, bodyFriction);

    mBodies.push_back(defaultPlaneBody);
    mShapes.push_back(Shape);

    mNumEntitiesInstanced++;

    return defaultPlaneBody;
}
// ------------------------------------------------------------------------- 
bool BaseApplication::checkIfEnoughPlaceToAddObject(float maxDist)
{
    //Ogre::Vector3 pickPos;
    //Ogre::Ray rayTo;
    //OgreBulletDynamics::RigidBody * body = 
    //    getBodyUnderCursorUsingBullet(pickPos, rayTo);
    //    //getBodyUnderCursorUsingOgre(pickPos, rayTo);
    //if (body)
    //{          
    //    if ((pickPos - mCamera->getDerivedPosition ()).length () < maxDist)
    //        return false;
    //}
    return true;        
}
// -------------------------------------------------------------------------
void BaseApplication::throwDynamicObject(OIS::KeyCode key)
{
    const float trowDist = 2.0f;
    switch(key)
    {
    case OIS::KC_V: 
        if ( checkIfEnoughPlaceToAddObject(trowDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCube("cube", vec, Ogre::Quaternion(0,0,0,1), 
                gCubeBodyBounds, gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

            body->setLinearVelocity(
                mCamera->getDerivedDirection().normalisedCopy() * mShootSpeed
				);
		}
        break;
    case OIS::KC_B: 
        if ( checkIfEnoughPlaceToAddObject(trowDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addSphere("sphere", vec, Ogre::Quaternion(0,0,0,1), 
                gSphereBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

            body->setLinearVelocity(
                mCamera->getDerivedDirection().normalisedCopy() * mShootSpeed
                );
        }
        break;
    case OIS::KC_N: 
        if ( checkIfEnoughPlaceToAddObject(trowDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCylinder("cylinder", vec, Ogre::Quaternion(0,0,0,1), 
                gCylinderBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

            body->setLinearVelocity(
                mCamera->getDerivedDirection().normalisedCopy() * mShootSpeed
                );
        }
        break;
    case OIS::KC_M : 
        if ( checkIfEnoughPlaceToAddObject(trowDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCone("cone", vec, Ogre::Quaternion(0,0,0,1), 
                gConeBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

            body->setLinearVelocity(
                mCamera->getDerivedDirection().normalisedCopy() * mShootSpeed
                );
        }
        break;
    }
}
// -------------------------------------------------------------------------
void BaseApplication::dropDynamicObject(OIS::KeyCode key)
{
    const float dropDist = 10.0f;
    switch(key)
    {
    case OIS::KC_J: 
        if ( checkIfEnoughPlaceToAddObject(dropDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCube("cube", 
                vec + mCamera->getDerivedDirection().normalisedCopy() * 10, 
                Ogre::Quaternion(0,0,0,1), 
                gCubeBodyBounds, gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

        }
        break;
    case OIS::KC_K: 
        if ( checkIfEnoughPlaceToAddObject(dropDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addSphere("sphere", 
                vec + mCamera->getDerivedDirection().normalisedCopy() * 10, 
                Ogre::Quaternion(0,0,0,1), 
                gSphereBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

        }
        break;
    case OIS::KC_U : 
        if ( checkIfEnoughPlaceToAddObject(dropDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCylinder("Cylinder", vec, Ogre::Quaternion(0,0,0,1), 
                gCylinderBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);

        }
        break;
    case OIS::KC_I: 
        if ( checkIfEnoughPlaceToAddObject(dropDist))
        {
            const Ogre::Vector3 vec (mCamera->getDerivedPosition());
            OgreBulletDynamics::RigidBody *body = addCone("Cone", 
                vec + mCamera->getDerivedDirection().normalisedCopy() * 10, 
                Ogre::Quaternion(0,0,0,1), 
                gConeBodyBounds, 
                gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass);
        }
        break;
    }
}
// -------------------------------------------------------------------------