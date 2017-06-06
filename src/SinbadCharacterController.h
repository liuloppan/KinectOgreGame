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
#include "AxisLines.h"

class SinbadCharacterController
{

public:
    SinbadCharacterController();
    virtual ~SinbadCharacterController();

    virtual void setupCharacter(Ogre::SceneManager *mSceneManager, KinectController *controller);
    virtual void updatePerFrame(Ogre::Real elapsedTime);
    Ogre::SceneNode *getEntityNode();

    Ogre::String entityName;

protected:
    void transformBone(Ogre::String boneName, NuiManager::NuiJointIndex jointIdx);

    void setupBone(const Ogre::String &name, NuiManager::NuiJointIndex idx);
    void setupBone(const Ogre::String &name, const Ogre::Radian &angle, const Ogre::Vector3 axis);
    void setupBone(const Ogre::String &name, const Ogre::Degree &yaw, const Ogre::Degree &pitch, const Ogre::Degree &roll);
    void setupBone(const Ogre::String &name, const Ogre::Quaternion &q);

    KinectController  *controller;
    JointOrientationCalculator *jointCalc;

    std::vector<AxisLines *> axisLines;

    bool showBoneOrientationAxes;
    bool showJointYAxes;

    Ogre::Real skelCenter;
    Ogre::Vector3 bodyOffset;

    Ogre::SceneManager *mSceneManager;
    Ogre::Entity *bodyEntity;
	Ogre::Entity* mSword1;
	Ogre::Entity* mSword2;
    Ogre::SceneNode *bodyNode;
    Ogre::Skeleton *skeleton;
};

#endif
