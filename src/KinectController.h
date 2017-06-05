
#ifndef __KinectController_h_
#define __KinectController_h_

#include "Ogre.h"
#include "KinectManager.h"

using namespace NuiManager;

/*
	Bridging kinect wrapper with C++ app
*/
class KinectController
{
public:
	KinectController(void);
	virtual ~KinectController(void);

	virtual void initController(void);
	virtual void unInitController(void);
	virtual void updatePerFrame(Ogre::Real elapsedTime);

	Ogre::Vector4 convertToOgreVector4(Vector4 nuiVector);
	Ogre::Vector3 convertToOgreVector3(Vector4 nuiVector);

	NUI_SKELETON_DATA* getSkeletonData();
	Ogre::Vector3 getJointPosition(NuiManager::NuiJointIndex idx);
	NuiSkeletonPositionTrackingState getJointStatus(NuiManager::NuiJointIndex idx);
	NuiSkeletonTrackingState getSkeletonStatus();

	void showColorData(Ogre::TexturePtr texRenderTarget);

protected:
	int player1Index;
	int player2Index;

	NuiSkeletonTrackingState player1State;
	NuiSkeletonTrackingState player2State;

	NUI_SKELETON_DATA* player1Skeleton;

	KinectManager* kinectManager;

	Ogre::SceneManager *mSceneManager;
};

#endif