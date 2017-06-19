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
    this->bodyOffset = Ogre::Vector3(0, 0, 0);
    this->mNumEntitiesInstanced = 0;
}
//-------------------------------------------------------------------------------------
SinbadCharacterController::~SinbadCharacterController()
{
	delete rbSinbad;
}
// -------------------------------------------------------------------------
void SinbadCharacterController::setupCharacter(Ogre::SceneManager *mSceneManager, KinectController *controller,OgreBulletDynamics::DynamicsWorld *mWorld)
{
    this->mSceneManager = mSceneManager;
    this->controller = controller;
	this->mWorld = mWorld;	

    jointCalc = new JointOrientationCalculator();
    jointCalc->setupController(controller);

    entityName = "MainBody";

    this->bodyEntity = mSceneManager->createEntity(entityName, "Sinbad.mesh");
    this->bodyNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
    this->bodyNode->attachObject(bodyEntity);
    this->bodyNode->scale(Ogre::Vector3(5));
    this->bodyNode->setPosition(bodyOffset);


	OgreBulletCollisions::BoxCollisionShape *colShapeSinbad = new OgreBulletCollisions::BoxCollisionShape( Ogre::Vector3 (10.f, 10.f, 10.f));
    // and the Bullet rigid body
    rbSinbad = new OgreBulletDynamics::RigidBody("rbSinbad", mWorld);
    rbSinbad->setShape(   bodyNode,
                    colShapeSinbad,
                    0.6f,         // dynamic body restitution
                    0.6f,         // dynamic body friction
                    0.0f,          // dynamic bodymass
                    bodyNode->getPosition(),      // starting position of the box
                    bodyNode->getOrientation());// orientation of the box

    // create swords and attach to sheath
    mSword1 = mSceneManager->createEntity("SinbadSword1", "Sword.mesh");
    mSword2 = mSceneManager->createEntity("SinbadSword2", "Sword.mesh");
    bodyEntity->attachObjectToBone("Sheath.L", mSword1);
    bodyEntity->attachObjectToBone("Sheath.R", mSword2);

    skeleton = this->bodyEntity->getSkeleton();
    skeleton->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);


    if (!jointCalc->getMirror()) {
        setupBone("Thigh.R",           NuiJointIndex::HIP_RIGHT);
        setupBone("Thigh.L",           NuiJointIndex::HIP_LEFT);
        setupBone("Calf.R",               NuiJointIndex::KNEE_RIGHT);
        setupBone("Calf.L",               NuiJointIndex::KNEE_LEFT);
        setupBone("Root",              NuiJointIndex::CENTER_HIP);
        setupBone("Neck",              NuiJointIndex::HEAD);
        setupBone("Humerus.R",            NuiJointIndex::SHOULDER_RIGHT);
        setupBone("Humerus.L",            NuiJointIndex::SHOULDER_LEFT);
        setupBone("Ulna.R",               NuiJointIndex::ELBOW_RIGHT);
        setupBone("Ulna.L",               NuiJointIndex::ELBOW_LEFT);
    } else {
        setupBone("Thigh.R",           NuiJointIndex::HIP_RIGHT);
        setupBone("Thigh.L",           NuiJointIndex::HIP_LEFT);
        setupBone("Calf.R",               NuiJointIndex::KNEE_LEFT);
        setupBone("Calf.L",               NuiJointIndex::KNEE_RIGHT);
        setupBone("Root",              NuiJointIndex::CENTER_HIP);
        setupBone("Neck",              NuiJointIndex::HEAD);
        setupBone("Humerus.R",            NuiJointIndex::SHOULDER_LEFT);
        setupBone("Humerus.L",            NuiJointIndex::SHOULDER_RIGHT);
        setupBone("Ulna.R",               NuiJointIndex::ELBOW_LEFT);
        setupBone("Ulna.L",               NuiJointIndex::ELBOW_RIGHT);
    }
    setupAnimations();
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setupAnimations()
{
    bodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
    Ogre::String animNames[] = {
        "IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
        "SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"
    };

    // populate our animation list
    for (int i = 0; i < NUM_ANIMS; i++) {
        mAnims[i] = bodyEntity->getAnimationState(animNames[i]);
        mAnims[i]->setLoop(true);
        mFadingIn[i] = false;
        mFadingOut[i] = false;
    }

    // start off in the idle state (top and bottom together)
    setBaseAnimation(ANIM_IDLE_BASE);
    setTopAnimation(ANIM_IDLE_TOP);

    // relax the hands since we're not holding anything
    mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);

    mSwordsDrawn = false;
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setBaseAnimation(AnimID id, bool reset)
{
    if (mBaseAnimID >= 0 && mBaseAnimID < NUM_ANIMS) {
        // if we have an old animation, fade it out
        mFadingIn[mBaseAnimID] = false;
        mFadingOut[mBaseAnimID] = true;
    }

    mBaseAnimID = id;

    if (id != ANIM_NONE) {
        // if we have a new animation, enable it and fade it in
        mAnims[id]->setEnabled(true);
        mAnims[id]->setWeight(0);
        mFadingOut[id] = false;
        mFadingIn[id] = true;
        if (reset) {
            mAnims[id]->setTimePosition(0);
        }
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::setTopAnimation(AnimID id, bool reset)
{
    if (mTopAnimID >= 0 && mTopAnimID < NUM_ANIMS) {
        // if we have an old animation, fade it out
        mFadingIn[mTopAnimID] = false;
        mFadingOut[mTopAnimID] = true;
    }

    mTopAnimID = id;

    if (id != ANIM_NONE) {
        // if we have a new animation, enable it and fade it in
        mAnims[id]->setEnabled(true);
        mAnims[id]->setWeight(0);
        mFadingOut[id] = false;
        mFadingIn[id] = true;
        if (reset) {
            mAnims[id]->setTimePosition(0);
        }
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::fadeAnimations(Ogre::Real deltaTime)
{
    for (int i = 0; i < NUM_ANIMS; i++) {
        if (mFadingIn[i]) {
            // slowly fade this animation in until it has full weight
            Ogre::Real newWeight = mAnims[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
            mAnims[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
            if (newWeight >= 1) {
                mFadingIn[i] = false;
            }
        } else if (mFadingOut[i]) {
            // slowly fade this animation out until it has no weight, and then disable it
            Ogre::Real newWeight = mAnims[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
            mAnims[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
            if (newWeight <= 0) {
                mAnims[i]->setEnabled(false);
                mFadingOut[i] = false;
            }
        }
    }
}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::updatePerFrame(Ogre::Real elapsedTime)
{
    using namespace NuiManager;

    if (controller->getSkeletonStatus() != NuiSkeletonTrackingState::SKELETON_TRACKED) {
        return;
    }

    Ogre::Real baseAnimSpeed = 1;
    Ogre::Real topAnimSpeed = 1;
    mTimer += elapsedTime;
    Ogre::Vector3 leftHand = controller->getJointPosition(HAND_LEFT);
    Ogre::Vector3 rightHand = controller->getJointPosition(HAND_RIGHT);
    Ogre::Vector3 Head = controller->getJointPosition(HEAD);
    std::ostringstream lstr, hstr;
    lstr << leftHand.z;
    hstr << Head.z;
    std::string mylefthand = lstr.str();
    std::string myhead = hstr.str();
    Ogre::LogManager::getSingletonPtr()->logMessage("HEAD");
    Ogre::LogManager::getSingletonPtr()->logMessage(myhead);
    Ogre::LogManager::getSingletonPtr()->logMessage("LEFT HAND");
    Ogre::LogManager::getSingletonPtr()->logMessage(mylefthand);
    if (leftHand.z >= 1.46071f) {
        setTopAnimation(ANIM_DRAW_SWORDS, true);
    }

    if (mTopAnimID == ANIM_DRAW_SWORDS) {
        // flip the draw swords animation if we need to put it back
        topAnimSpeed = mSwordsDrawn ? -1 : 1;

        bodyEntity->detachAllObjectsFromBone();
        bodyEntity->attachObjectToBone(mSwordsDrawn ? "Sheath.L" : "Handle.L", mSword1);
        bodyEntity->attachObjectToBone(mSwordsDrawn ? "Sheath.R" : "Handle.R", mSword2);

        // change the hand state to grab or let go
        mAnims[ANIM_HANDS_CLOSED]->setEnabled(!mSwordsDrawn);
        mAnims[ANIM_HANDS_RELAXED]->setEnabled(mSwordsDrawn);
    }

    if (!jointCalc->getMirror()) {
        transformBone("Thigh.R",          NuiJointIndex::HIP_RIGHT);
        transformBone("Thigh.L",          NuiJointIndex::HIP_LEFT);
        transformBone("Calf.R",              NuiJointIndex::KNEE_RIGHT);
        transformBone("Calf.L",              NuiJointIndex::KNEE_LEFT);
        transformBone("Root",             NuiJointIndex::CENTER_HIP);
        transformBone("Neck",             NuiJointIndex::HEAD);
        transformBone("Humerus.R",           NuiJointIndex::SHOULDER_RIGHT);
        transformBone("Humerus.L",           NuiJointIndex::SHOULDER_LEFT);
        transformBone("Ulna.R",              NuiJointIndex::ELBOW_RIGHT);
        transformBone("Ulna.L",              NuiJointIndex::ELBOW_LEFT);

    } else {
        transformBone("Thigh.L",          NuiJointIndex::HIP_RIGHT);
        transformBone("Thigh.R",          NuiJointIndex::HIP_LEFT);
        transformBone("Calf.L",              NuiJointIndex::KNEE_RIGHT);
        transformBone("Calf.R",              NuiJointIndex::KNEE_LEFT);
        transformBone("Root",             NuiJointIndex::CENTER_HIP);
        transformBone("Neck",             NuiJointIndex::HEAD);
        transformBone("Humerus.R",           NuiJointIndex::SHOULDER_LEFT);
        transformBone("Humerus.L",           NuiJointIndex::SHOULDER_RIGHT);
        transformBone("Ulna.R",              NuiJointIndex::ELBOW_LEFT);
        transformBone("Ulna.L",              NuiJointIndex::ELBOW_RIGHT);

    }
	

}
//-------------------------------------------------------------------------------------
void SinbadCharacterController::transformBone(Ogre::String boneName, NuiManager::NuiJointIndex jointIdx)
{
    int state = 0;
    state = (int)controller->getJointStatus(jointIdx);
    btTransform tr;
    if (state == 2) {
        Ogre::Bone *bone = skeleton->getBone(boneName);
        Ogre::Quaternion qI = bone->getInitialOrientation();
        Ogre::Quaternion newQ = jointCalc->getSkeletonJointOrientation(jointIdx);

        if (boneName == "Root") {
            //kinect skeleton position is in meter 0.8m<z<4m
			Ogre::Vector3 newBodyPos = controller->getJointPosition(jointIdx) * 60.0f;
			newBodyPos.z = -1 * newBodyPos.z;
			newBodyPos += bodyOffset;

			bodyNode->setPosition(newBodyPos);
			rbSinbad->setPosition(bodyNode->getPosition());
        }
        bone->resetOrientation();
        newQ = bone->convertWorldToLocalOrientation(newQ);
        bone->setOrientation(newQ * qI);

        Ogre::Quaternion resQ = bone->getOrientation();

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