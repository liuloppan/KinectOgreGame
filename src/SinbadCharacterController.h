/*
-----------------------------------------------------------------------------
Filename:    SinbadCharacterController.h
-----------------------------------------------------------------------------
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
#ifndef __SinbadCharacterController_H__
#define __SinbadCharacterController_H__

#include "Ogre.h"
#include "KinectController.h"
#include "JointOrientationCalculator.h"
#include "OgreMatrix3.h"
//#include "btBulletDynamicsCommon.h"
#include "OgreBulletDynamics.h"

#define NUM_ANIMS 13
#define ANIM_FADE_SPEED 7.5f   // animation crossfade speed in % of full weight per second

class SinbadCharacterController
{

public:

    SinbadCharacterController();

    virtual ~SinbadCharacterController();

    virtual void setupCharacter(Ogre::SceneManager *mSceneManager, KinectController *controller, OgreBulletDynamics::DynamicsWorld *mWorld);
    virtual void updatePerFrame(Ogre::Real elapsedTime);
    Ogre::SceneNode *getEntityNode();

    Ogre::String entityName;

protected:
    enum AnimID {
        ANIM_IDLE_BASE,
        ANIM_IDLE_TOP,
        ANIM_RUN_BASE,
        ANIM_RUN_TOP,
        ANIM_HANDS_CLOSED,
        ANIM_HANDS_RELAXED,
        ANIM_DRAW_SWORDS,
        ANIM_SLICE_VERTICAL,
        ANIM_SLICE_HORIZONTAL,
        ANIM_DANCE,
        ANIM_JUMP_START,
        ANIM_JUMP_LOOP,
        ANIM_JUMP_END,
        ANIM_NONE
    };
    void transformBone(Ogre::String boneName, NuiManager::NuiJointIndex jointIdx);

    void setupBone(const Ogre::String &name, NuiManager::NuiJointIndex idx);
    void setupBone(const Ogre::String &name, const Ogre::Radian &angle, const Ogre::Vector3 axis);
    void setupBone(const Ogre::String &name, const Ogre::Degree &yaw, const Ogre::Degree &pitch, const Ogre::Degree &roll);
    void setupBone(const Ogre::String &name, const Ogre::Quaternion &q);
    void setTopAnimation(AnimID id, bool reset = false);
    void setBaseAnimation(AnimID id, bool reset = false);
    void fadeAnimations(Ogre::Real deltaTime);
    void setupAnimations();

    KinectController  *controller;
    JointOrientationCalculator *jointCalc;

    bool showBoneOrientationAxes;
    bool showJointYAxes;

    Ogre::Real skelCenter;
    Ogre::Vector3 bodyOffset;

    OgreBulletDynamics::DynamicsWorld *mWorld;
    OgreBulletDynamics::RigidBody *rbSinbad;
    OgreBulletDynamics::RigidBody *rbHandL;
    Ogre::SceneManager *mSceneManager;
    Ogre::Entity *bodyEntity;
    Ogre::Entity *mSword1;
    Ogre::Entity *mSword2;
    Ogre::SceneNode *bodyNode;
    Ogre::SceneNode *handNodeL;
    Ogre::Real handRadius;
    Ogre::Skeleton *skeleton;
    AnimID mBaseAnimID;                   // current base (full- or lower-body) animation
    AnimID mTopAnimID;                    // current top (upper-body) animation
    bool mFadingIn[NUM_ANIMS];            // which animations are fading in
    bool mFadingOut[NUM_ANIMS];           // which animations are fading out
    Ogre::AnimationState *mAnims[NUM_ANIMS];    // master animation list
    bool mSwordsDrawn;
    Ogre::Real mTimer;                // general timer to see how long animations have been playing
    //btRigidBody *SindabRigidBody;
    int mNumEntitiesInstanced;
    OgreBulletDynamics::DynamicsWorld *mDynamicsWorld;
    std::deque<Ogre::Entity *>                          mEntities;
    std::deque<OgreBulletDynamics::RigidBody *>         mBodies;
    OgreBulletCollisions::CapsuleCollisionShape *colShapeSinbad;
};

#endif
