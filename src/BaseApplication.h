/*
-----------------------------------------------------------------------------
Filename:    BaseApplication.h
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
#ifndef __BaseApplication_h_
#define __BaseApplication_h_

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include "OgreBulletDynamics.h"
#include "OgreBulletCollisions.h"

enum QueryFlags
{
	ANY_QUERY_MASK					= 1<<0,
	RAGDOLL_QUERY_MASK				= 1<<1,
	GEOMETRY_QUERY_MASK				= 1<<2,
	VEHICLE_QUERY_MASK				= 1<<3,
	STATIC_GEOMETRY_QUERY_MASK		= 1<<4
};

class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    BaseApplication(void);
    virtual ~BaseApplication(void);

    virtual void go(void);

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0; // Override me!
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

    // OIS::KeyListener
    virtual bool keyPressed(const OIS::KeyEvent &arg);
    virtual bool keyReleased(const OIS::KeyEvent &arg);
    // OIS::MouseListener
    virtual bool mouseMoved(const OIS::MouseEvent &arg);
    virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow *rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow *rw);

    Ogre::Root *mRoot;
    Ogre::Camera *mCamera;
    Ogre::SceneManager *mSceneMgr;
    Ogre::RenderWindow *mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    // OgreBites
    OgreBites::SdkTrayManager *mTrayMgr;
    OgreBites::SdkCameraMan *mCameraMan;       // basic camera controller
    OgreBites::ParamsPanel *mDetailsPanel;     // sample details panel
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager *mInputManager;
    OIS::Mouse    *mMouse;
    OIS::Keyboard *mKeyboard;

	// bullet
	bool checkIfEnoughPlaceToAddObject(float maxDist);
    void throwDynamicObject(OIS::KeyCode key);
    void dropDynamicObject(OIS::KeyCode key);
	OgreBulletDynamics::RigidBody* getBodyUnderCursorUsingBullet(Ogre::Vector3 &intersectionPoint, Ogre::Ray &rayTo);
    OgreBulletDynamics::RigidBody* getBodyUnderCursorUsingOgre(Ogre::Vector3 &intersectionPoint, Ogre::Ray &rayTo);

	void initWorld (const Ogre::Vector3 &gravityVector = Ogre::Vector3 (0,-9.81,0), 
                    const Ogre::AxisAlignedBox &bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),
                                                                               Ogre::Vector3 (10000,  10000,  10000)));

    void addGround();

    OgreBulletDynamics::RigidBody *addCube(const Ogre::String instanceName,
        const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
        const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
        const Ogre::Real bodyMass);

    OgreBulletDynamics::RigidBody *addCylinder(const Ogre::String instanceName,
        const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
        const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
        const Ogre::Real bodyMass);

    OgreBulletDynamics::RigidBody *addSphere(const Ogre::String instanceName,
        const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Real radius,
        const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
        const Ogre::Real bodyMass);


    OgreBulletDynamics::RigidBody *addCone(const Ogre::String instanceName,
        const Ogre::Vector3 &pos, const Ogre::Quaternion &q, const Ogre::Vector3 &size,
        const Ogre::Real bodyRestitution, const Ogre::Real bodyFriction, 
        const Ogre::Real bodyMass);

    OgreBulletDynamics::RigidBody *addStaticTrimesh(const Ogre::String &instanceName,
                                                        const Ogre::String &meshName,
                                                        const Ogre::Vector3 &pos, 
                                                        const Ogre::Quaternion &q, 
                                                        const Ogre::Real bodyRestitution, 
                                                        const Ogre::Real bodyFriction,
                                                        const bool castShadow = true);

    OgreBulletDynamics::RigidBody *addStaticPlane( const Ogre::Real bodyRestitution, 
                                                   const Ogre::Real bodyFriction);

	OgreBulletDynamics::DynamicsWorld *mWorld;
	std::deque<OgreBulletDynamics::RigidBody *>         mBodies;
    std::deque<OgreBulletCollisions::CollisionShape *>  mShapes;
	std::deque<Ogre::Entity *>                          mEntities;
	static size_t mNumEntitiesInstanced;
	float                    mShootSpeed;
};

#endif // #ifndef __BaseApplication_h_
