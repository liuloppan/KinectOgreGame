
#include "Stdafx.h"
#include "KinectController.h"

//-------------------------------------------------------------------------------------
KinectController::KinectController(void)
	: kinectManager(0),
	player1Skeleton(0),
	player1Index(-1),
	player2Index(-1),
	player1State(NuiSkeletonTrackingState::SKELETON_NOT_TRACKED),
	player2State(NuiSkeletonTrackingState::SKELETON_NOT_TRACKED)
{
}

//-------------------------------------------------------------------------------------
KinectController::~KinectController(void)
{	
}

//-------------------------------------------------------------------------------------
void KinectController::initController(void)
{
	kinectManager = new KinectManager();
	kinectManager->InitNui();
}

//-------------------------------------------------------------------------------------
void KinectController::unInitController(void)
{
	//if(skeleton) delete skeleton;

	if(kinectManager)
	{
		kinectManager->UnInitNui();
		delete kinectManager;
	}
}

//-------------------------------------------------------------------------------------
void KinectController::updatePerFrame(Ogre::Real elapsedTime)
{
	if(player1State == NuiSkeletonTrackingState::SKELETON_NOT_TRACKED)		// not tracked
	{
		// find new player
		for(int i = 0; i < NUI_SKELETON_COUNT; i++)
		{
			player1Skeleton = kinectManager->getSkeleton(i);
			if((NuiSkeletonTrackingState)player1Skeleton->eTrackingState == NuiSkeletonTrackingState::SKELETON_TRACKED)
			{
				std::cout << "index: " << i << "\n";
				player1Index = i;
				player1State = NuiSkeletonTrackingState::SKELETON_TRACKED;
				break;
			}
		}
	}
	else	// tracked
	{
		player1Skeleton = kinectManager->getSkeleton(player1Index);
		player1State = (NuiSkeletonTrackingState)player1Skeleton->eTrackingState;
	}
}

//-------------------------------------------------------------------------------------
NuiSkeletonTrackingState KinectController::getSkeletonStatus()
{
	NuiSkeletonTrackingState state = (NuiSkeletonTrackingState)kinectManager->getSkeleton(player1Index)->eTrackingState;
	return state;
}

//-------------------------------------------------------------------------------------
void KinectController::showColorData(Ogre::TexturePtr texRenderTarget)
{	
}

//-------------------------------------------------------------------------------------
NUI_SKELETON_DATA* KinectController::getSkeletonData()
{
	return player1Skeleton;
}

//-------------------------------------------------------------------------------------
Ogre::Vector4 KinectController::convertToOgreVector4(Vector4 nuiVector)
{
	return Ogre::Vector4(nuiVector.x, nuiVector.y, nuiVector.z, nuiVector.w);
}

//-------------------------------------------------------------------------------------
Ogre::Vector3 KinectController::convertToOgreVector3(Vector4 nuiVector)
{
	return Ogre::Vector3(nuiVector.x, nuiVector.y, nuiVector.z);
}

//-------------------------------------------------------------------------------------
Ogre::Vector3 KinectController::getJointPosition(NuiManager::NuiJointIndex idx)
{
	Ogre::Vector3 jointPos = this->convertToOgreVector3(this->getSkeletonData()->SkeletonPositions[idx]);
	//jointPos.x *= -1;

	return jointPos;

	//return this->convertToOgreVector3(this->getSkeletonData()->SkeletonPositions[idx]);
}

//-------------------------------------------------------------------------------------
NuiSkeletonPositionTrackingState KinectController::getJointStatus(NuiManager::NuiJointIndex idx)
{
	return (NuiSkeletonPositionTrackingState)this->getSkeletonData()->eSkeletonPositionTrackingState[idx];
}