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
#include "sdkTrays.h"

#include <ole2.h>
#include <NuiApi.h>

#define SKELETON_BONE_COUNT	20

class OgreKinectGame : public BaseApplication
{
public:
    OgreKinectGame();
    virtual ~OgreKinectGame();
    virtual void go(void);
    virtual bool frameStarted(const Ogre::FrameEvent &evt);

protected:
    virtual void createScene();
    virtual void destroyScene();
    virtual bool frameRenderingQueued(const Ogre::FrameEvent &fe);
    virtual bool keyPressed(const OIS::KeyEvent &evt);
    virtual bool keyReleased(const OIS::KeyEvent &evt);
    virtual bool mouseMoved(const OIS::MouseEvent &evt);
    virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
    virtual void buttonHit(OgreBites::Button *b);


    // setting up the scene
    virtual void checkBoxToggled(OgreBites::CheckBox *box);
    virtual void itemSelected(OgreBites::SelectMenu *menu);
    virtual bool setup();
    virtual void setupWidgets();
    virtual void setupGPUforKinect(void);
    virtual void setupKinect(void);
    virtual HRESULT createKinect(void);
    virtual void drawBone(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex, Ogre::Vector2 *jointPoints, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1, const int &boneIndex);
    virtual void drawSkeleton(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex);
    virtual void manualRender(void);
private:
    void setupToggles();
    void setMenuVisible(const Ogre::String &name, bool visible = true);
    void setupParticles();

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

    bool mMenuName;
    Ogre::Entity *mFloor;
    Ogre::Light *mLight;
    SinbadCharacterController *mChara;
    Ogre::NameValuePairList mInfo;
};

#endif // #ifndef __OgreKinectGame_h_