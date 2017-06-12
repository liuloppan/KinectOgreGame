
#ifndef __KinectController_h_
#define __KinectController_h_

#include "Ogre.h"
#include "KinectManager.h"

using namespace NuiManager;

/*
	Bridging kinect wrapper with C++ app
*/
#define SKELETON_BONE_COUNT	20


class KinectController
{
public:
    KinectController(Ogre::SceneManager *aSceneMgr);
    virtual ~KinectController(void);

    virtual void initController(void);
    virtual void unInitController(void);
    virtual void updatePerFrame(Ogre::Real elapsedTime);

    Ogre::Vector4 convertToOgreVector4(Vector4 nuiVector);
    Ogre::Vector3 convertToOgreVector3(Vector4 nuiVector);
    Ogre::SceneManager *mSceneManager;

    NUI_SKELETON_DATA *getSkeletonData();	
    void showColorData(Ogre::TexturePtr texRenderTarget);

	//stuff needed to draw skeleton
    Ogre::Vector3 getJointPosition(NuiManager::NuiJointIndex idx);
    NuiSkeletonPositionTrackingState getJointStatus(NuiManager::NuiJointIndex idx);
    NuiSkeletonTrackingState getSkeletonStatus();

	Ogre::ManualObject* mSkeletonBone[NUI_SKELETON_COUNT][SKELETON_BONE_COUNT];
	Ogre::SceneNode* mBoneNode[NUI_SKELETON_COUNT][SKELETON_BONE_COUNT];

	Ogre::ManualObject* mSkeletonJoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];
	Ogre::SceneNode* mJointNode[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];

	void showSkeletonViewPort();
	void drawSkeleton(const NUI_SKELETON_DATA& skeletonData, const int& skeletonIndex);
	void drawBone(const NUI_SKELETON_DATA& skeletonData, const int& skeletonIndex, Ogre::Vector2* jointPoint, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1, const int& boneIndex);
	void setupSkeletonDraw();
	void setupGPUSkeletonDraw(void);

	virtual void manualRender(void);

	DWORD mColorWidth, mColorHeight, mDepthWidth, mDepthHeight;
	HANDLE mColorEvent, mColorStream, mDepthEvent, mDepthStream, mSkeletonEvent;

	Ogre::uint16* mDepthImage;
	Ogre::Rectangle2D* mDepthRect;
	Ogre::SceneNode* mDepthRectNode;

	Ogre::Overlay* mOverlay;
	Ogre::TexturePtr mDepthTarget;
	Ogre::TexturePtr mDepthTex;



	static const NUI_IMAGE_RESOLUTION mDepthResolution = NUI_IMAGE_RESOLUTION_640x480;

protected:
    int player1Index;
    int player2Index;

    NuiSkeletonTrackingState player1State;
    NuiSkeletonTrackingState player2State;

    NUI_SKELETON_DATA *player1Skeleton;

    KinectManager *kinectManager;

};

#endif