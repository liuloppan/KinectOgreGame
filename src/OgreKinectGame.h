/*
-----------------------------------------------------------------------------
Filename:    OgreKinectGame.h
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
#ifndef __OgreKinectGame_h_
#define __OgreKinectGame_h_

#include "BaseApplication.h"
#include "SinbadCharacterController.h"
#include "KinectController.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "SkeletonToRagdoll.h"
#include "OgreDisplay.h"

#include "sdkTrays.h"
using namespace OgreBites;

class OgreKinectGame : public BaseApplication
{
public:
    OgreKinectGame();
    virtual ~OgreKinectGame();

protected:
    virtual void createScene();
    virtual void destroyScene();
    virtual bool frameRenderingQueued(const Ogre::FrameEvent &fe);
    virtual bool mouseMoved(const OIS::MouseEvent &evt);
    virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
    virtual bool keyReleased(const OIS::KeyEvent &evt);
    virtual void setupKinect(void);
	virtual bool setup();
	virtual void buttonHit(Button *b);

    void setupWidgets();
    void gameOver();
    void createBall(Ogre::Real time);

protected:
    Ogre::Entity	*mFloor;
    Ogre::Light	*mLight;
    SinbadCharacterController	*character;
    Ogre::NameValuePairList	mInfo;
    KinectController	*kinectController;
    Ogre::TexturePtr	texRenderTarget;
    OgreDisplay							*ogreDisplay;
    btBroadphaseInterface					*broadphase;
    btCollisionDispatcher					*dispatcher;
    btSequentialImpulseConstraintSolver	*solver;
    btDefaultCollisionConfiguration		*collisionConfiguration;
    btDynamicsWorld						*dynamicsWorld;
    SkeletonToRagdoll						*ragdoll;
    double									accumulator;
    const double							dt;
    Ogre::Timer *timer;
    Ogre::String timerString;
    int									 numBalls;
    Ogre::Real							 mTimeSinceLastBall;
    long gameTime; // how long the game lasts for in milliseconds
    OgreBites::Label *timerLabel;
    OgreBites::Label *scoreLabel;
};

#endif // #ifndef __OgreKinectGame_h_