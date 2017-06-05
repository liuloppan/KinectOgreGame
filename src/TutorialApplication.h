/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.h
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
#ifndef __TutorialApplication_h_
#define __TutorialApplication_h_

#include "BaseApplication.h"
#include "SinbadCharacterController.h"

#include <ole2.h>
#include <NuiApi.h>


#define SKELETON_BONE_COUNT	20

class TutorialApplication : public BaseApplication
{
public:
    TutorialApplication(void);
    virtual ~TutorialApplication(void);

    virtual void go(void);
    virtual bool frameStarted(const Ogre::FrameEvent &evt);

protected:
    virtual bool setup(void);
    virtual void createScene(void);
    virtual void setupGPUforKinect(void);
    virtual void setupKinect(void);
    virtual HRESULT createKinect(void);

    virtual void drawBone(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex, Ogre::Vector2 *jointPoints, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1, const int &boneIndex);
    virtual void drawSkeleton(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex);
    virtual void manualRender(void);

    DWORD mColorWidth, mColorHeight, mDepthWidth, mDepthHeight;
    HANDLE mColorEvent, mColorStream, mDepthEvent, mDepthStream, mSkeletonEvent;

    Ogre::uint8 *mColorImage;
    Ogre::uint16 *mDepthImage;
    NUI_SKELETON_FRAME mSkeletonData;

    Ogre::ManualObject *mSkeletonBone[NUI_SKELETON_COUNT][SKELETON_BONE_COUNT];
    Ogre::SceneNode *mBoneNode[NUI_SKELETON_COUNT][SKELETON_BONE_COUNT];

    Ogre::ManualObject *mSkeletonJoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];
    Ogre::SceneNode *mJointNode[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT];

    Ogre::Rectangle2D *mColorRect;
    Ogre::Rectangle2D *mDepthRect;

    Ogre::SceneNode *mColorRectNode;
    Ogre::SceneNode *mDepthRectNode;

    Ogre::TexturePtr mColorTarget;
    Ogre::TexturePtr mDepthTarget;

    Ogre::Overlay *mOverlay;
    Ogre::TexturePtr mColorTex;
    Ogre::TexturePtr mDepthTex;

    static const int mBytesPerPixel = 4;
    static const NUI_IMAGE_RESOLUTION mColorResolution = NUI_IMAGE_RESOLUTION_640x480;
    static const NUI_IMAGE_RESOLUTION mDepthResolution = NUI_IMAGE_RESOLUTION_640x480;

	SinbadCharacterController *mCharSinbad;
};

#endif // #ifndef __TutorialApplication_h_
