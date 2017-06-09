/*
-----------------------------------------------------------------------------
Filename:    SinbadCharacterController.cpp
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

#include "Stdafx.h"
#include "SinbadCharacterController.h"

//-------------------------------------------------------------------------------------
SinbadCharacterController::SinbadCharacterController()
{
    this->showBoneOrientationAxes = false;

    this->skelCenter = 10000.0f;
    this->bodyOffset = Ogre::Vector3(0, 25, 0);
}
//-------------------------------------------------------------------------------------
SinbadCharacterController::~SinbadCharacterController()
{
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupCharacter(Ogre::SceneManager *mSceneManager, KinectController *controller)
{
    this->mSceneManager = mSceneManager;
    this->controller = controller;

    jointCalc = new JointOrientationCalculator();
    jointCalc->setupController(controller);

    entityName = "MainBody";

    this->bodyEntity = mSceneManager->createEntity(entityName, "Sinbad.mesh");
    this->bodyNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
    this->bodyNode->attachObject(bodyEntity);
    this->bodyNode->scale(Ogre::Vector3(5));
    this->bodyNode->setPosition(bodyOffset);

    // create swords and attach to sheath
    mSword1 = mSceneManager->createEntity("SinbadSword1", "Sword.mesh");
    mSword2 = mSceneManager->createEntity("SinbadSword2", "Sword.mesh");
    bodyEntity->attachObjectToBone("Sheath.L", mSword1);
    bodyEntity->attachObjectToBone("Sheath.R", mSword2);

    skeleton = this->bodyEntity->getSkeleton();
    skeleton->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

    for (int a = 0; a < NUI_SKELETON_POSITION_COUNT && showBoneOrientationAxes; a++) {	// debug
        AxisLines *axl = new AxisLines();
        axisLines.push_back(axl);
        axl->length = 3;

    }

    if (!jointCalc->getMirror()) {
        setupBone("Thigh.R",				NuiJointIndex::HIP_RIGHT);
        setupBone("Thigh.L",				NuiJointIndex::HIP_LEFT);
        setupBone("Calf.R",					NuiJointIndex::KNEE_RIGHT);
        setupBone("Calf.L",					NuiJointIndex::KNEE_LEFT);
        setupBone("Root",					NuiJointIndex::CENTER_HIP);
        setupBone("Neck",					NuiJointIndex::HEAD);
        //setupBone("Clavicle.L",				NuiJointIndex::CENTER_SHOULDER);
        //setupBone("Clavicle.R",				NuiJointIndex::CENTER_SHOULDER);
        setupBone("Humerus.R",				NuiJointIndex::SHOULDER_RIGHT);
        setupBone("Humerus.L",				NuiJointIndex::SHOULDER_LEFT);
        setupBone("Ulna.R",					NuiJointIndex::ELBOW_RIGHT);
        setupBone("Ulna.L",					NuiJointIndex::ELBOW_LEFT);
    } else {
        setupBone("Thigh.R",				NuiJointIndex::HIP_RIGHT);
        setupBone("Thigh.L",				NuiJointIndex::HIP_LEFT);
        setupBone("Calf.R",					NuiJointIndex::KNEE_LEFT);
        setupBone("Calf.L",					NuiJointIndex::KNEE_RIGHT);
        setupBone("Root",					NuiJointIndex::CENTER_HIP);
        setupBone("Neck",					NuiJointIndex::HEAD);
        //setupBone("Clavicle.L",				NuiJointIndex::CENTER_SHOULDER);
        //setupBone("Clavicle.R",				NuiJointIndex::CENTER_SHOULDER);
        setupBone("Humerus.R",				NuiJointIndex::SHOULDER_LEFT);
        setupBone("Humerus.L",				NuiJointIndex::SHOULDER_RIGHT);
        setupBone("Ulna.R",					NuiJointIndex::ELBOW_LEFT);
        setupBone("Ulna.L",					NuiJointIndex::ELBOW_RIGHT);
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::updatePerFrame(Ogre::Real elapsedTime)
{
    using namespace NuiManager;

    if (controller->getSkeletonStatus() != NuiSkeletonTrackingState::SKELETON_TRACKED) {
        return;
    }

    Ogre::Real yLeftSoulder = controller->getJointPosition(SHOULDER_LEFT).y;
    Ogre::Real yRightShoulder = controller->getJointPosition(SHOULDER_RIGHT).y;
    Ogre::Real yCurSkelCenter = (yLeftSoulder < yRightShoulder) ? yLeftSoulder : yRightShoulder;
    //Ogre::Real shoulderPos = (lShoulderPos <= rShoulderPos) ? lShoulderPos : rShoulderPos;
    //Ogre::Real curSkelHeight = shoulderPos;
    //Ogre::Real curSkelHeight = controller->getJointPosition(SHOULDER_CENTER).y + shoulderPos / 2.0f;

    Ogre::Real dif = yCurSkelCenter - skelCenter;
    Ogre::Vector3 bodyPos = bodyNode->getPosition();

    if (dif <= -25 || dif > 25) {
    } else if (dif >= 0.02f || dif <= -0.02f) {
        Ogre::Real yTranslation = dif * 10000 * elapsedTime;
        if ((yTranslation + bodyOffset.y) < bodyOffset.y) {
            yTranslation = 0;
        }
        bodyNode->translate(Ogre::Vector3(0, yTranslation, 0));
    } else {
        if (bodyPos.y <= (bodyOffset.y + 0.1f) && bodyPos.y >= (bodyOffset.y - 0.1f)) {
            bodyNode->setPosition(bodyOffset);
        } else if (bodyPos.y > bodyOffset.y) {
            bodyNode->translate(Ogre::Vector3(0, -50 * elapsedTime, 0));
        } else if (bodyPos.y < bodyOffset.y) {
            bodyNode->translate(Ogre::Vector3(0, 100 * elapsedTime, 0));
        }
    }

    skelCenter = yCurSkelCenter;


    if (!jointCalc->getMirror()) {
        transformBone("Thigh.R",				NuiJointIndex::HIP_RIGHT);
        transformBone("Thigh.L",				NuiJointIndex::HIP_LEFT);
        transformBone("Calf.R",					NuiJointIndex::KNEE_RIGHT);
        transformBone("Calf.L",					NuiJointIndex::KNEE_LEFT);
        transformBone("Root",					NuiJointIndex::CENTER_HIP);
        transformBone("Neck",					NuiJointIndex::HEAD);
        //transformBone("Clavicle.L",				NuiJointIndex::CENTER_SHOULDER);
        //transformBone("Clavicle.R",				NuiJointIndex::CENTER_SHOULDER);
        transformBone("Humerus.R",				NuiJointIndex::SHOULDER_RIGHT);
        transformBone("Humerus.L",				NuiJointIndex::SHOULDER_LEFT);
        transformBone("Ulna.R",					NuiJointIndex::ELBOW_RIGHT);
        transformBone("Ulna.L",					NuiJointIndex::ELBOW_LEFT);
    } else {
        transformBone("Thigh.L",				NuiJointIndex::HIP_RIGHT);
        transformBone("Thigh.R",				NuiJointIndex::HIP_LEFT);
        transformBone("Calf.L",					NuiJointIndex::KNEE_RIGHT);
        transformBone("Calf.R",					NuiJointIndex::KNEE_LEFT);
        transformBone("Root",					NuiJointIndex::CENTER_HIP);
        transformBone("Neck",					NuiJointIndex::HEAD);
        //transformBone("Clavicle.L",				NuiJointIndex::CENTER_SHOULDER);
        //transformBone("Clavicle.R",				NuiJointIndex::CENTER_SHOULDER);
        transformBone("Humerus.R",				NuiJointIndex::SHOULDER_LEFT);
        transformBone("Humerus.L",				NuiJointIndex::SHOULDER_RIGHT);
        transformBone("Ulna.R",					NuiJointIndex::ELBOW_LEFT);
        transformBone("Ulna.L",					NuiJointIndex::ELBOW_RIGHT);
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::transformBone(Ogre::String boneName, NuiManager::NuiJointIndex jointIdx)
{
    int state = 0;
    state = (int)controller->getJointStatus(jointIdx);

    if (state == 2) {
        Ogre::Bone *bone = skeleton->getBone(boneName);
        Ogre::Quaternion qI = bone->getInitialOrientation();
        Ogre::Quaternion newQ = jointCalc->getSkeletonJointOrientation(jointIdx);

        bone->resetOrientation();
        newQ = bone->convertWorldToLocalOrientation(newQ);
        bone->setOrientation(newQ * qI);

        Ogre::Quaternion resQ = bone->getOrientation();
        if (showBoneOrientationAxes) {
            axisLines[jointIdx]->updateLines(resQ.xAxis(), resQ.yAxis(), resQ.zAxis());    // debug
        }
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupBone(const Ogre::String &name, NuiJointIndex idx)
{
    Ogre::Bone *bone = bodyEntity->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);

    if (name != "Humerus.R" && name != "Humerus.L") {
        bone->setInheritOrientation(true);
    }

    bone->resetOrientation();

    Ogre::Quaternion q = Ogre::Quaternion::IDENTITY;
    bone->setOrientation(q);

    bone->setInitialState();

    if (showBoneOrientationAxes) {
        axisLines[idx]->initAxis(name, this->bodyEntity, this->mSceneManager);    // debug
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupBone(const Ogre::String &name, const Ogre::Radian &angle, const Ogre::Vector3 axis)
{
    Ogre::Quaternion q;
    q.FromAngleAxis(angle, axis);
    setupBone(name, q);
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupBone(const Ogre::String &name, const Ogre::Degree &yaw, const Ogre::Degree &pitch, const Ogre::Degree &roll)
{
    Ogre::Bone *bone = bodyEntity->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);

    bone->resetOrientation();

    bone->yaw(yaw);
    bone->pitch(pitch);
    bone->roll(roll);
    bone->setInitialState();
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupBone(const Ogre::String &name, const Ogre::Quaternion &q)
{
    Ogre::Bone *bone = bodyEntity->getSkeleton()->getBone(name);
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);

    bone->resetOrientation();
    bone->setOrientation(q);

    bone->setInitialState();
}
//-------------------------------------------------------------------------------------
Ogre::SceneNode *SinbadCharacterController::getEntityNode()
{
    return this->bodyNode;
}