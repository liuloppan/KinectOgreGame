/*
-----------------------------------------------------------------------------
Filename:    OgreKinectGame.cpp
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
#include "OgreKinectGame.h"
bool isUIvisible = false;

//-------------------------------------------------------------------------------------
OgreKinectGame::OgreKinectGame()
{
    mInfo["About"] = "Ogre Kinect Game @2017.\n"
                     "Created for 3D Game Programming at Computer Scicence Yuan Ze University\n"
                     "Developer :\n"
                     "Lovisa Hassler\n"
                     "Rosdyana Kusuma\n"
                     "Project available on : https://github.com/liuloppan/KinectOgreGame/";
    mColorImage	= NULL;
    mDepthImage	= NULL;

    mColorRect = NULL;
    mDepthRect = NULL;

    mColorEvent		= INVALID_HANDLE_VALUE;
    mColorStream	= INVALID_HANDLE_VALUE;
    mDepthEvent		= INVALID_HANDLE_VALUE;
    mDepthStream	= INVALID_HANDLE_VALUE;

    mSkeletonEvent	= INVALID_HANDLE_VALUE;
}
//-------------------------------------------------------------------------------------
OgreKinectGame::~OgreKinectGame()
{
    NuiShutdown();

    CloseHandle(mColorEvent);
    CloseHandle(mDepthEvent);
    CloseHandle(mSkeletonEvent);

    if (mColorRect != NULL) {
        delete mColorRect;
    }
    if (mDepthRect != NULL) {
        delete mDepthRect;
    }

    if (mColorImage != NULL) {
        delete [] mColorImage;
    }
    if (mDepthImage != NULL) {
        delete [] mDepthImage;
    }
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup()) {
        return;
    }

    while (!mShutDown) {
        manualRender();
        Ogre::WindowEventUtilities::messagePump();
    }

    // Clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mouseMoved(const OIS::MouseEvent &evt)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectMouseMove(evt);
    }
    return BaseApplication::mouseMoved(evt);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectMouseDown(evt, id);
    }
    return BaseApplication::mousePressed(evt, id);
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyPressed(const OIS::KeyEvent &evt)
{
    // relay input events to character controller
    if (!mTrayMgr->isDialogVisible()) {
        if (evt.key == OIS::KC_ESCAPE) {
            if (!isUIvisible) {
                mTrayMgr->showAll();
                setMenuVisible("MainMenu");
                setMenuVisible("Option", false);
                mTrayMgr->showCursor();
                isUIvisible = true;
            } else {
                setMenuVisible("Option", false);
                setMenuVisible("MainMenu", false);
                mTrayMgr->hideCursor();
                isUIvisible = false;
            }
        }
        if (!isUIvisible) {
            mChara->injectKeyDown(evt);
        }
    }
    return true;
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::keyReleased(const OIS::KeyEvent &evt)
{
    // relay input events to character controller
    if (!isUIvisible) {
        mChara->injectKeyUp(evt);
    }
    return BaseApplication::keyReleased(evt);
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::buttonHit(OgreBites::Button *b)
{
    if (b->getName() == "mQuitButton") {
        mRoot->queueEndRendering();
    } else if (b->getName() == "mOptionButton") {
        setMenuVisible("Option");
        setMenuVisible("MainMenu", false);
    } else if (b->getName() == "mCreditButton") {
        mTrayMgr->showOkDialog("About", mInfo["About"]);
    }
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::setup(void)
{

    if (!BaseApplication::setup()) {
        return false;
    }
    // Load fonts for tray captions
    Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
    setupKinect();
    setupWidgets();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::setupGPUforKinect(void)
{
    // Create render target textures for processing and drawing images from Kinect
    Ogre::TextureManager &textureManager = Ogre::TextureManager::getSingleton();
    mColorTarget = textureManager.createManual("ColorTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mColorWidth, mColorHeight,
                   0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);
    mDepthTarget = textureManager.createManual("DepthTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mDepthWidth, mDepthHeight,
                   0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);

    // Create cameras for processing images from Kinect
    Ogre::Camera *colorCam = mSceneMgr->createCamera("ColorCam");
    colorCam->setAspectRatio(static_cast<Ogre::Real>(mColorWidth) / static_cast<Ogre::Real>(mColorHeight));

    Ogre::Viewport *colorViewport = mColorTarget->getBuffer()->getRenderTarget()->addViewport(colorCam);
    colorViewport->setClearEveryFrame(true);
    colorViewport->setBackgroundColour(Ogre::ColourValue::Black);
    colorViewport->setOverlaysEnabled(false);

    Ogre::Camera *depthCam = mSceneMgr->createCamera("DepthCam");
    depthCam->setAspectRatio(static_cast<Ogre::Real>(mDepthWidth) / static_cast<Ogre::Real>(mDepthHeight));

    Ogre::Viewport *depthViewport = mDepthTarget->getBuffer()->getRenderTarget()->addViewport(depthCam);
    depthViewport->setClearEveryFrame(true);
    depthViewport->setBackgroundColour(Ogre::ColourValue::Black);
    depthViewport->setOverlaysEnabled(false);


    // Create materials for processing images from Kinect
    Ogre::MaterialManager &materialManager = Ogre::MaterialManager::getSingleton();
    if (!materialManager.resourceExists("Kinect/Color")) {
        Ogre::MaterialPtr material = materialManager.create("Kinect/Color", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_REPLACE);
        pass->createTextureUnitState("ColorTexture");
    }

    if (!materialManager.resourceExists("Kinect/Depth")) {
        Ogre::MaterialPtr material = materialManager.create("Kinect/Depth", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_REPLACE);
        pass->createTextureUnitState("DepthTexture");
    }


    // Create 2D planes for processing images from Kinect
    Ogre::Rectangle2D *mColorRect = new Ogre::Rectangle2D(true);
    mColorRect->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
    mColorRect->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));
    mColorRect->setMaterial("Kinect/Color");

    mColorRectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("ColorRectNode");
    mColorRectNode->attachObject(mColorRect);

    Ogre::Rectangle2D *mDepthRect = new Ogre::Rectangle2D(true);
    mDepthRect->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
    mDepthRect->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));
    mDepthRect->setMaterial("Kinect/Depth");

    mDepthRectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("DepthRectNode");
    mDepthRectNode->attachObject(mDepthRect);
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::setupKinect(void)
{
    // Connect to a Kinect device
    HRESULT hr = createKinect();
    if (FAILED(hr)) {
        return;
    }


    // Get image resolutions
    NuiImageResolutionToSize(mColorResolution, mColorWidth, mColorHeight);
    NuiImageResolutionToSize(mDepthResolution, mDepthWidth, mDepthHeight);

    // Create image data arrays
    mColorImage = new Ogre::uint8[mColorWidth * mColorHeight * mBytesPerPixel];
    mDepthImage = new Ogre::uint16[mDepthWidth * mDepthHeight];


    // Create textures for storing images from Kinect
    Ogre::TextureManager &textureManager = Ogre::TextureManager::getSingleton();
    mColorTex = textureManager.createManual("ColorTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mColorWidth, mColorHeight,
                                            0, Ogre::PF_X8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    mDepthTex = textureManager.createManual("DepthTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mDepthWidth, mDepthHeight,
                                            0, Ogre::PF_L16, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


    // Setup GPU resources for processing and drawing images from Kinect
    setupGPUforKinect();


    // Create materials for drawing images from Kinect
    Ogre::MaterialManager &materialManager = Ogre::MaterialManager::getSingleton();
    if (!materialManager.resourceExists("Overlay/Color")) {
        Ogre::MaterialPtr material = materialManager.create("Overlay/Color", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_ADD);
        pass->createTextureUnitState("ColorTarget");
    }

    if (!materialManager.resourceExists("Overlay/Depth")) {
        Ogre::MaterialPtr material = materialManager.create("Overlay/Depth", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_ADD);
        pass->createTextureUnitState("DepthTarget");
    }


    // Create an overlay for drawing images from Kinect
    Ogre::OverlayManager &overlayManager = Ogre::OverlayManager::getSingleton();
    mOverlay = overlayManager.create("KinectOverlay");

    // Create a panels for color and depth images
    Ogre::OverlayContainer *colorPanel = static_cast<Ogre::OverlayContainer *>(overlayManager.createOverlayElement("Panel", "ColorPanel"));
    colorPanel->setPosition(0.0, 0.0);
    colorPanel->setDimensions(0.5, 0.5);
    colorPanel->setMaterialName("Overlay/Color");

    Ogre::OverlayContainer *depthPanel = static_cast<Ogre::OverlayContainer *>(overlayManager.createOverlayElement("Panel", "DepthPanel"));
    depthPanel->setPosition(0.5, 0.0);
    depthPanel->setDimensions(0.5, 0.5);
    depthPanel->setMaterialName("Overlay/Depth");

    // Add panels to the overlay
    mOverlay->add2D(colorPanel);
    mOverlay->add2D(depthPanel);

    // Show the overlay
    mOverlay->show();


    // Create materials for drawing bones and joints
    if (!materialManager.resourceExists("Bone/Tracked")) {
        Ogre::MaterialPtr material = materialManager.create("Bone/Tracked", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setDepthCheckEnabled(false);
        pass->setDepthWriteEnabled(false);
        pass->setDiffuse(0.0, 0.0, 0.0, 1.0);
        pass->setAmbient(0.0, 0.0, 0.0);
        pass->setSelfIllumination(0.0, 0.5, 0.0);
    }

    if (!materialManager.resourceExists("Bone/Inferred")) {
        Ogre::MaterialPtr material = materialManager.create("Bone/Inferred", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setDepthCheckEnabled(false);
        pass->setDepthWriteEnabled(false);
        pass->setDiffuse(0.0, 0.0, 0.0, 1.0);
        pass->setAmbient(0.0, 0.0, 0.0);
        pass->setSelfIllumination(0.5, 0.5, 0.5);
    }

    if (!materialManager.resourceExists("Joint/Tracked")) {
        Ogre::MaterialPtr material = materialManager.create("Joint/Tracked", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setDepthCheckEnabled(false);
        pass->setDepthWriteEnabled(false);
        pass->setDiffuse(0.0, 0.0, 0.0, 1.0);
        pass->setAmbient(0.0, 0.0, 0.0);
        pass->setSelfIllumination(1.0, 1.0, 0.0);
        pass->setPointSize(5.0);
    }

    if (!materialManager.resourceExists("Joint/Inferred")) {
        Ogre::MaterialPtr material = materialManager.create("Joint/Inferred", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setDepthCheckEnabled(false);
        pass->setDepthWriteEnabled(false);
        pass->setDiffuse(0.0, 0.0, 0.0, 1.0);
        pass->setAmbient(0.0, 0.0, 0.0);
        pass->setSelfIllumination(0.25, 0.75, 0.25);
        pass->setPointSize(5.0);
    }


    // Create manual objects and scene nodes for drawing bones and joints
    Ogre::SceneNode *mRoot = mSceneMgr->getRootSceneNode();
    for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        for (int j = 0; j < SKELETON_BONE_COUNT; ++j) {
            Ogre::String indexName = Ogre::StringConverter::toString(i) + "_" + Ogre::StringConverter::toString(j);
            mSkeletonBone[i][j] = mSceneMgr->createManualObject("Bone" + indexName);
            mSkeletonBone[i][j]->setDynamic(true);
            mSkeletonBone[i][j]->setUseIdentityView(true);
            mSkeletonBone[i][j]->setUseIdentityProjection(true);

            mSkeletonBone[i][j]->begin("Bone/Tracked", Ogre::RenderOperation::OT_LINE_LIST);
            mSkeletonBone[i][j]->position(0.0, 0.0, 0.0);
            mSkeletonBone[i][j]->position(0.0, 0.0, 0.0);
            mSkeletonBone[i][j]->end();

            // Use infinite AAB to always stay visible
            mSkeletonBone[i][j]->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));

            // Render just after overlays
            mSkeletonBone[i][j]->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY + 1);

            mBoneNode[i][j] = mRoot->createChildSceneNode("BoneNode" + indexName);
            mBoneNode[i][j]->attachObject(mSkeletonBone[i][j]);
        }

        for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j) {
            Ogre::String indexName = Ogre::StringConverter::toString(i) + "_" + Ogre::StringConverter::toString(j);
            mSkeletonJoint[i][j] = mSceneMgr->createManualObject("Joint" + indexName);
            mSkeletonJoint[i][j]->setDynamic(true);
            mSkeletonJoint[i][j]->setUseIdentityView(true);
            mSkeletonJoint[i][j]->setUseIdentityProjection(true);

            mSkeletonJoint[i][j]->begin("Joint/Tracked", Ogre::RenderOperation::OT_POINT_LIST);
            mSkeletonJoint[i][j]->position(0.0, 0.0, 0.0);
            mSkeletonJoint[i][j]->end();

            // Use infinite AAB to always stay visible
            mSkeletonJoint[i][j]->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));

            // Render just after overlays
            mSkeletonJoint[i][j]->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY + 1);

            mJointNode[i][j] = mRoot->createChildSceneNode("JointNode" + indexName);
            mJointNode[i][j]->attachObject(mSkeletonJoint[i][j]);
        }
    }
}

//-------------------------------------------------------------------------------------
HRESULT OgreKinectGame::createKinect(void)
{
    // Initialize Kinect with color, depth, and skeleton data
    HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (FAILED(hr)) {
        return hr;
    }

    // Create event for color image
    mColorEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Open a stream for color image
    hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, mColorResolution, 0, 2, mColorEvent, &mColorStream);
    if (FAILED(hr)) {
        return hr;
    }

    // Create event for depth image
    mDepthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Open a stream for depth image
    hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, mDepthResolution, 0, 2, mDepthEvent, &mDepthStream);
    if (FAILED(hr)) {
        return hr;
    }

    // Create event for skeleton data
    mSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Enable skeletal tracking
    hr = NuiSkeletonTrackingEnable(mSkeletonEvent, 0);
    return hr;
}

//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameStarted(const Ogre::FrameEvent &evt)
{
    // Get color image from Kinect
    if (WaitForSingleObject(mColorEvent, 0) == WAIT_OBJECT_0) {
        const NUI_IMAGE_FRAME *colorImage;
        HRESULT hr = NuiImageStreamGetNextFrame(mColorStream, 0, &colorImage);

        if (SUCCEEDED(hr)) {
            NUI_LOCKED_RECT LockedRect;
            hr = colorImage->pFrameTexture->LockRect(0, &LockedRect, NULL, 0);

            if (SUCCEEDED(hr)) {
                memcpy(mColorImage, LockedRect.pBits, LockedRect.size);
                colorImage->pFrameTexture->UnlockRect(0);
            }

            NuiImageStreamReleaseFrame(mColorStream, colorImage);
        }
    }

    // Get depth image from Kinect
    if (WaitForSingleObject(mDepthEvent, 0) == WAIT_OBJECT_0) {
        const NUI_IMAGE_FRAME *depthImage;
        HRESULT hr = NuiImageStreamGetNextFrame(mDepthStream, 0, &depthImage);

        if (SUCCEEDED(hr)) {
            NUI_LOCKED_RECT LockedRect;
            hr = depthImage->pFrameTexture->LockRect(0, &LockedRect, NULL, 0);

            if (SUCCEEDED(hr)) {
                for (size_t i = 0; i < mDepthTex->getHeight(); ++i) {
                    Ogre::uint16 *pData = reinterpret_cast<Ogre::uint16 *>(&(LockedRect.pBits[i * LockedRect.Pitch]));
                    Ogre::uint16 *pImage = &mDepthImage[i * mDepthTex->getWidth()];
                    for (size_t j = 0; j < mDepthTex->getWidth(); ++j) {
                        pImage[j] = NuiDepthPixelToDepth(pData[j]);
                    }
                }
                depthImage->pFrameTexture->UnlockRect(0);
            }

            NuiImageStreamReleaseFrame(mDepthStream, depthImage);
        }
    }

    // Get skeleton data from Kinect
    if (WaitForSingleObject(mSkeletonEvent, 0) == WAIT_OBJECT_0) {
        HRESULT hr = NuiSkeletonGetNextFrame(0, &mSkeletonData);

        // Smooth out the skeleton data
        if (SUCCEEDED(hr)) {
            NuiTransformSmooth(&mSkeletonData, NULL);
        }
    }


    // Overlay color image
    Ogre::HardwarePixelBufferSharedPtr colorBuffer = mColorTex->getBuffer();
    colorBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
    const Ogre::PixelBox &colorBox = colorBuffer->getCurrentLock();

    // Copy color image data
    Ogre::uint8 *colorDataPtr = static_cast<Ogre::uint8 *>(colorBox.data);
    for (size_t i = 0; i < mColorTex->getHeight(); ++i) {
        memcpy(colorDataPtr, &mColorImage[i * mColorTex->getWidth() * mBytesPerPixel], mColorTex->getWidth() * mBytesPerPixel);
        colorDataPtr += colorBox.rowPitch * mBytesPerPixel;
    }

    colorBuffer->unlock();

    // Overlay depth image
    Ogre::HardwarePixelBufferSharedPtr depthBuffer = mDepthTex->getBuffer();
    depthBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
    const Ogre::PixelBox &depthBox = depthBuffer->getCurrentLock();

    // Copy color image data
    Ogre::uint16 *depthDataPtr = static_cast<Ogre::uint16 *>(depthBox.data);
    for (size_t i = 0; i < mDepthTex->getHeight(); ++i) {
        memcpy(depthDataPtr, &mDepthImage[i * mDepthTex->getWidth()], mDepthTex->getWidth() * sizeof(Ogre::uint16));
        depthDataPtr += depthBox.rowPitch;
    }

    depthBuffer->unlock();

    // Advance the animation
    //mSinbadState->addTime(evt.timeSinceLastFrame);
    return true;
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::drawBone(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex, Ogre::Vector2 *jointPoint, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1, const int &boneIndex)
{
    NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skeletonData.eSkeletonPositionTrackingState[joint0];
    NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skeletonData.eSkeletonPositionTrackingState[joint1];

    // Do not draw if either of the two joints is found
    if ((joint0State == NUI_SKELETON_POSITION_NOT_TRACKED) || (joint1State == NUI_SKELETON_POSITION_NOT_TRACKED)) {
        return;
    }

    // Do not draw if both joints are inferred
    if ((joint0State == NUI_SKELETON_POSITION_INFERRED) && (joint1State == NUI_SKELETON_POSITION_INFERRED)) {
        return;
    }

    // Set positions of endpoints of line
    mSkeletonBone[skeletonIndex][boneIndex]->beginUpdate(0);
    mSkeletonBone[skeletonIndex][boneIndex]->position(jointPoint[joint0].x, jointPoint[joint0].y, 0.0);
    mSkeletonBone[skeletonIndex][boneIndex]->position(jointPoint[joint1].x, jointPoint[joint1].y, 0.0);
    mSkeletonBone[skeletonIndex][boneIndex]->end();

    // Different colors for tracked and inferred bones
    if ((joint0State == NUI_SKELETON_POSITION_TRACKED) && (joint1State == NUI_SKELETON_POSITION_TRACKED)) {
        mSkeletonBone[skeletonIndex][boneIndex]->setMaterialName(0, "Bone/Tracked");
    } else {
        mSkeletonBone[skeletonIndex][boneIndex]->setMaterialName(0, "Bone/Inferred");
    }

    // Update scene graph
    mSceneMgr->getRootSceneNode()->addChild(mBoneNode[skeletonIndex][boneIndex]);
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::drawSkeleton(const NUI_SKELETON_DATA &skeletonData, const int &skeletonIndex)
{
    // Obtain joint positions on depth image
    Ogre::Vector2 jointPoint[NUI_SKELETON_POSITION_COUNT];
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
        NuiTransformSkeletonToDepthImage(skeletonData.SkeletonPositions[i], &(jointPoint[i].x), &(jointPoint[i].y), mDepthResolution);
        jointPoint[i].x = jointPoint[i].x / static_cast<float>(mDepthWidth);
        jointPoint[i].y = 1.0 - jointPoint[i].y / static_cast<float>(mDepthHeight);
    }

    // Draw torso
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER, 0);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, 1);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, 2);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE, 3);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER, 4);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, 5);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, 6);

    // Draw left arm
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, 7);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, 8);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT, 9);

    // Draw right arm
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, 10);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, 11);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT, 12);

    // Draw left leg
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, 13);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, 14);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT, 15);

    // Draw right leg
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, 16);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, 17);
    drawBone(skeletonData, skeletonIndex, jointPoint, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT, 18);

    // Draw joints in a different color
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
        if (skeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED) {
            // Set position and material of joint
            mSkeletonJoint[skeletonIndex][i]->beginUpdate(0);
            mSkeletonJoint[skeletonIndex][i]->position(jointPoint[i].x, jointPoint[i].y, 0.0);
            mSkeletonJoint[skeletonIndex][i]->end();
            mSkeletonJoint[skeletonIndex][i]->setMaterialName(0, "Joint/Tracked");

            // Update scene graph
            mSceneMgr->getRootSceneNode()->addChild(mJointNode[skeletonIndex][i]);
        } else if (skeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED) {
            // Set position and material of joint
            mSkeletonJoint[skeletonIndex][i]->beginUpdate(0);
            mSkeletonJoint[skeletonIndex][i]->position(jointPoint[i].x, jointPoint[i].y, 0.0);
            mSkeletonJoint[skeletonIndex][i]->end();
            mSkeletonJoint[skeletonIndex][i]->setMaterialName(0, "Joint/Inferred");

            // Update scene graph
            mSceneMgr->getRootSceneNode()->addChild(mJointNode[skeletonIndex][i]);
        }
    }
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::manualRender(void)
{
    // Manually call pre-rendering callbacks
    mRoot->_fireFrameStarted();
    mRoot->_fireFrameRenderingQueued();

    // Update scene graph for processing color image from Kinect
    Ogre::SceneNode *rootNode = mSceneMgr->getRootSceneNode();
    rootNode->removeAllChildren();
    rootNode->addChild(mColorRectNode);
    mColorTarget->getBuffer()->getRenderTarget()->update();

    // Update scene graph for processing depth image from Kinect
    rootNode->removeAllChildren();
    rootNode->addChild(mDepthRectNode);
    mDepthTarget->getBuffer()->getRenderTarget()->update();


    // Update scene graph for rendering the scene
    rootNode->removeAllChildren();
    rootNode->addChild(mChara->mBodyNode);

    // Render skeleton data
    for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE trackState = mSkeletonData.SkeletonData[i].eTrackingState;
        if (trackState == NUI_SKELETON_TRACKED) {
            drawSkeleton(mSkeletonData.SkeletonData[i], i);
        } else if (trackState == NUI_SKELETON_POSITION_ONLY) {
            // Obtain skeleton position on depth image
            Ogre::Vector2 jointPoint;
            NuiTransformSkeletonToDepthImage(mSkeletonData.SkeletonData[i].Position, &(jointPoint.x), &(jointPoint.y));
            jointPoint.x = jointPoint.x / static_cast<float>(mDepthWidth);
            jointPoint.y = 1.0 - jointPoint.y / static_cast<float>(mDepthHeight);

            // Set position and material for drawing skeleton position
            mSkeletonJoint[i][0]->beginUpdate(0);
            mSkeletonJoint[i][0]->position(jointPoint.x, jointPoint.y, 0.0);
            mSkeletonJoint[i][0]->end();
            mSkeletonJoint[i][0]->setMaterialName(0, "Joint/Tracked");

            // Update scene graph
            mSceneMgr->getRootSceneNode()->addChild(mJointNode[i][0]);
        }
    }

    mWindow->update();

    // Manually call post-rendering callback
    mRoot->_fireFrameEnded();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::createScene()
{
    // setup some basic lighting for our scene
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
    mLight = mSceneMgr->createLight();
    mLight->setPosition(20, 20, 50);
    // disable default camera control so the character can do its own
    mCameraMan->setStyle(OgreBites::CS_MANUAL);
    mChara = new SinbadCharacterController(mCamera);

    // create a floor mesh resource
    Ogre::MeshManager::getSingleton().createPlane("floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, -18), 10000, 10000, 20, 20, true, 1, 128, 128, Ogre::Vector3::UNIT_Z);


    // create a floor entity, give it a material, and place it at the origin
    mFloor = mSceneMgr->createEntity("Floor", "floor");
    mFloor->setMaterialName("Examples/BeachStones");
    mSceneMgr->getRootSceneNode()->attachObject(mFloor);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 10, 8);
}

//-------------------------------------------------------------------------------------
void OgreKinectGame::setMenuVisible(const Ogre::String &name, bool visible)
{
    if (name == "MainMenu") {
        if (visible) {
            mTrayMgr->moveWidgetToTray("mMainMenuLabel", OgreBites::TL_CENTER);
            mTrayMgr->moveWidgetToTray("mOptionButton", OgreBites::TL_CENTER);
            mTrayMgr->moveWidgetToTray("mCreditButton", OgreBites::TL_CENTER);
            mTrayMgr->moveWidgetToTray("mQuitButton", OgreBites::TL_CENTER);
            mTrayMgr->getWidget("mMainMenuLabel")->show();
            mTrayMgr->getWidget("mOptionButton")->show();
            mTrayMgr->getWidget("mCreditButton")->show();
            mTrayMgr->getWidget("mQuitButton")->show();
        } else {
            mTrayMgr->removeWidgetFromTray("mMainMenuLabel");
            mTrayMgr->removeWidgetFromTray("mOptionButton");
            mTrayMgr->removeWidgetFromTray("mCreditButton");
            mTrayMgr->removeWidgetFromTray("mQuitButton");
            mTrayMgr->getWidget("mMainMenuLabel")->hide();
            mTrayMgr->getWidget("mOptionButton")->hide();
            mTrayMgr->getWidget("mCreditButton")->hide();
            mTrayMgr->getWidget("mQuitButton")->hide();
        }
    }
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::destroyScene()
{
    if (mChara) {
        delete mChara;
        mChara = 0;
    }
    if (mTrayMgr) {
        mTrayMgr->destroyAllWidgets();
    }
    Ogre::MeshManager::getSingleton().remove("floor");

    mSceneMgr->clearScene(); // removes all nodes, billboards, lights etc.
    mSceneMgr->destroyAllCameras();
}
//-------------------------------------------------------------------------------------
bool OgreKinectGame::frameRenderingQueued(const Ogre::FrameEvent &fe)
{
    mChara->addTime(fe.timeSinceLastFrame);
    mTrayMgr->frameRenderingQueued(fe);
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    return true;
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::setupWidgets()
{
    mTrayMgr->destroyAllWidgets();
    // create check boxes to toggle the visibility of our particle systems
    const int WIDTH_UI = 160;

    // main menu
    mTrayMgr->createLabel(OgreBites::TL_NONE, "mMainMenuLabel", "Main Menu", WIDTH_UI);
    mTrayMgr->createButton(OgreBites::TL_NONE, "mOptionButton", "Option");
    mTrayMgr->createButton(OgreBites::TL_NONE, "mCreditButton", "About");
    mTrayMgr->createButton(OgreBites::TL_NONE, "mQuitButton", "Quit");

    mTrayMgr->hideAll();
}
//-------------------------------------------------------------------------------------
void OgreKinectGame::checkBoxToggled(OgreBites::CheckBox *box)
{

}
//-------------------------------------------------------------------------------------
void OgreKinectGame::itemSelected(OgreBites::SelectMenu *menu)
{
    Ogre::String currentElement = menu->getSelectedItem();

}
//-------------------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char *argv[])
#endif
{
    // Create application object
    OgreKinectGame app;

    try {
        app.go();
    } catch (Ogre::Exception &e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
                  e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}

#ifdef __cplusplus
}
#endif