
#include "Stdafx.h"
#include "KinectController.h"

//-------------------------------------------------------------------------------------
KinectController::KinectController(Ogre::SceneManager *aSceneMgr)
    : kinectManager(0),
      player1Skeleton(0),
      player1Index(-1),
      player2Index(-1),
      player1State(NuiSkeletonTrackingState::SKELETON_NOT_TRACKED),
      player2State(NuiSkeletonTrackingState::SKELETON_NOT_TRACKED),
	  mSceneManager(aSceneMgr)
{
}

//-------------------------------------------------------------------------------------
KinectController::~KinectController(void)
{
	//NuiShutdown();

   // CloseHandle(mDepthEvent);
  //  CloseHandle(mSkeletonEvent);


	if(mDepthRect != NULL) delete mDepthRect;

	if(mDepthImage != NULL) delete [] mDepthImage;
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

    if (kinectManager) {
        kinectManager->UnInitNui();
        delete kinectManager;
    }
}

//-------------------------------------------------------------------------------------
void KinectController::updatePerFrame(Ogre::Real elapsedTime)
{
    if (player1State == NuiSkeletonTrackingState::SKELETON_NOT_TRACKED) {	// not tracked
        // find new player
        for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
            player1Skeleton = kinectManager->getSkeleton(i);
            if ((NuiSkeletonTrackingState)player1Skeleton->eTrackingState == NuiSkeletonTrackingState::SKELETON_TRACKED) {
                std::cout << "index: " << i << "\n";
                player1Index = i;
                player1State = NuiSkeletonTrackingState::SKELETON_TRACKED;
                break;
            }
        }
    } else {	// tracked
        player1Skeleton = kinectManager->getSkeleton(player1Index);
        player1State = (NuiSkeletonTrackingState)player1Skeleton->eTrackingState;


	}
}
//-------------------------------------------------------------------------------------
	void KinectController::manualRender()
{
	// Manually call pre-rendering callbacks
	Ogre::SceneNode* mRoot = mSceneManager->getRootSceneNode();
	mRoot->_fireFrameStarted();
	mRoot->_fireFrameRenderingQueued();

	// Update scene graph for processing color image from Kinect
	Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode();
	rootNode->removeAllChildren();
	rootNode->addChild(mColorRectNode);
	mColorTarget->getBuffer()->getRenderTarget()->update();

	// Update scene graph for processing depth image from Kinect
	rootNode->removeAllChildren();
	rootNode->addChild(mDepthRectNode);
	mDepthTarget->getBuffer()->getRenderTarget()->update();


	// Update scene graph for rendering the scene
	//rootNode->removeAllChildren();
	//rootNode->addChild(mSinbadNode);

	// Render skeleton data
	for(int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackState = mSkeletonData.SkeletonData[i].eTrackingState;
		if(trackState == NUI_SKELETON_TRACKED)
			drawSkeleton(mSkeletonData.SkeletonData[i], i);
		else if(trackState == NUI_SKELETON_POSITION_ONLY)
		{
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
NUI_SKELETON_DATA *KinectController::getSkeletonData()
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

//-------------------------------------------------------------------------------------

void KinectController::showSkeletonViewPort()
{
	//NUI_SKELETON_DATA *copySkeletonData = new _NUI_SKELETON_DATA(*player1Skeleton);
	setupSkeletonDraw();
	

}



void KinectController::drawBone(const NUI_SKELETON_DATA& skeletonData, const int& skeletonIndex, Ogre::Vector2* jointPoint, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1, const int& boneIndex)
{
    NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skeletonData.eSkeletonPositionTrackingState[joint0];
    NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skeletonData.eSkeletonPositionTrackingState[joint1];

    // Do not draw if either of the two joints is found
    if((joint0State == NUI_SKELETON_POSITION_NOT_TRACKED) || (joint1State == NUI_SKELETON_POSITION_NOT_TRACKED))
        return;
    
    // Do not draw if both joints are inferred
    if((joint0State == NUI_SKELETON_POSITION_INFERRED) && (joint1State == NUI_SKELETON_POSITION_INFERRED))
        return;

    // Set positions of endpoints of line
	mSkeletonBone[skeletonIndex][boneIndex]->beginUpdate(0);
	mSkeletonBone[skeletonIndex][boneIndex]->position(jointPoint[joint0].x, jointPoint[joint0].y, 0.0);
	mSkeletonBone[skeletonIndex][boneIndex]->position(jointPoint[joint1].x, jointPoint[joint1].y, 0.0);
	mSkeletonBone[skeletonIndex][boneIndex]->end();

    // Different colors for tracked and inferred bones
    if((joint0State == NUI_SKELETON_POSITION_TRACKED) && (joint1State == NUI_SKELETON_POSITION_TRACKED))
		mSkeletonBone[skeletonIndex][boneIndex]->setMaterialName(0, "Bone/Tracked");
    else
		mSkeletonBone[skeletonIndex][boneIndex]->setMaterialName(0, "Bone/Inferred");

    // Update scene graph
	mSceneManager->getRootSceneNode()->addChild(mBoneNode[skeletonIndex][boneIndex]);
}

//-------------------------------------------------------------------------------------
void KinectController::drawSkeleton(const NUI_SKELETON_DATA& skeletonData, const int& skeletonIndex)
{
	// Obtain joint positions on depth image
	Ogre::Vector2 jointPoint[NUI_SKELETON_POSITION_COUNT];
    for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
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
    for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
    {
		if(skeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED)
		{
			// Set position and material of joint
			mSkeletonJoint[skeletonIndex][i]->beginUpdate(0);
			mSkeletonJoint[skeletonIndex][i]->position(jointPoint[i].x, jointPoint[i].y, 0.0);
			mSkeletonJoint[skeletonIndex][i]->end();
			mSkeletonJoint[skeletonIndex][i]->setMaterialName(0, "Joint/Tracked");

			// Update scene graph
			mSceneManager->getRootSceneNode()->addChild(mJointNode[skeletonIndex][i]);
		}
		else if(skeletonData.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED)
		{
			// Set position and material of joint
			mSkeletonJoint[skeletonIndex][i]->beginUpdate(0);
			mSkeletonJoint[skeletonIndex][i]->position(jointPoint[i].x, jointPoint[i].y, 0.0);
			mSkeletonJoint[skeletonIndex][i]->end();
			mSkeletonJoint[skeletonIndex][i]->setMaterialName(0, "Joint/Inferred");

			// Update scene graph
			mSceneManager->getRootSceneNode()->addChild(mJointNode[skeletonIndex][i]);
		}
    }
}

void KinectController::setupSkeletonDraw()
{
	// Connect to a Kinect device
	if(!kinectManager)
		return;

	// Get image resolutions
    //NuiImageResolutionToSize(mColorResolution, mColorWidth, mColorHeight);
    NuiImageResolutionToSize(mDepthResolution, mDepthWidth, mDepthHeight);

	// Create image data arrays
	//mColorImage = new Ogre::uint8[mColorWidth * mColorHeight * mBytesPerPixel];
	mDepthImage = new Ogre::uint16[mDepthWidth * mDepthHeight];


	// Create textures for storing images from Kinect
	Ogre::TextureManager& textureManager = Ogre::TextureManager::getSingleton();
	//mColorTex = textureManager.createManual("ColorTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mColorWidth, mColorHeight,
//											0, Ogre::PF_X8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
	mDepthTex = textureManager.createManual("DepthTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mDepthWidth, mDepthHeight,
											0, Ogre::PF_L16, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


	// Setup GPU resources for processing and drawing images from Kinect
	setupGPUSkeletonDraw();


	// Create materials for drawing images from Kinect
	Ogre::MaterialManager& materialManager = Ogre::MaterialManager::getSingleton();
	//if(!materialManager.resourceExists("Overlay/Color"))
	//{
	//	Ogre::MaterialPtr material = materialManager.create("Overlay/Color", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//	Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
	//	pass->setSceneBlending(Ogre::SBT_ADD);
	//	pass->createTextureUnitState("ColorTarget");
	//}

	if(!materialManager.resourceExists("Overlay/Depth"))
	{
		Ogre::MaterialPtr material = materialManager.create("Overlay/Depth", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_ADD);
		pass->createTextureUnitState("DepthTarget");
	}


	// Create an overlay for drawing images from Kinect
	Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
	mOverlay = overlayManager.create("KinectOverlay");

	// Create a panels for color and depth images
	//Ogre::OverlayContainer* colorPanel = static_cast<Ogre::OverlayContainer *>(overlayManager.createOverlayElement("Panel", "ColorPanel"));
	//colorPanel->setPosition(0.0, 0.0);
	//colorPanel->setDimensions(0.5, 0.5);
	//colorPanel->setMaterialName("Overlay/Color");

	Ogre::OverlayContainer* depthPanel = static_cast<Ogre::OverlayContainer *>(overlayManager.createOverlayElement("Panel", "DepthPanel"));
	depthPanel->setPosition(0.5, 0.0);
	depthPanel->setDimensions(0.5, 0.5);
	depthPanel->setMaterialName("Overlay/Depth");

	// Add panels to the overlay
	//mOverlay->add2D(colorPanel);
	mOverlay->add2D(depthPanel);

	// Show the overlay
	mOverlay->show();


	// Create materials for drawing bones and joints
	if(!materialManager.resourceExists("Bone/Tracked"))
	{
		Ogre::MaterialPtr material = materialManager.create("Bone/Tracked", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthCheckEnabled(false);
		pass->setDepthWriteEnabled(false);
		pass->setDiffuse(0.0, 0.0, 0.0, 1.0); 
		pass->setAmbient(0.0, 0.0, 0.0); 
		pass->setSelfIllumination(0.0, 0.5, 0.0);
	}

	if(!materialManager.resourceExists("Bone/Inferred"))
	{
		Ogre::MaterialPtr material = materialManager.create("Bone/Inferred", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthCheckEnabled(false);
		pass->setDepthWriteEnabled(false);
		pass->setDiffuse(0.0, 0.0, 0.0, 1.0); 
		pass->setAmbient(0.0, 0.0, 0.0); 
		pass->setSelfIllumination(0.5, 0.5, 0.5);
	}

	if(!materialManager.resourceExists("Joint/Tracked"))
	{
		Ogre::MaterialPtr material = materialManager.create("Joint/Tracked", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthCheckEnabled(false);
		pass->setDepthWriteEnabled(false);
		pass->setDiffuse(0.0, 0.0, 0.0, 1.0); 
		pass->setAmbient(0.0, 0.0, 0.0); 
		pass->setSelfIllumination(1.0, 1.0, 0.0);
		pass->setPointSize(5.0);
	}

	if(!materialManager.resourceExists("Joint/Inferred"))
	{
		Ogre::MaterialPtr material = materialManager.create("Joint/Inferred", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthCheckEnabled(false);
		pass->setDepthWriteEnabled(false);
		pass->setDiffuse(0.0, 0.0, 0.0, 1.0); 
		pass->setAmbient(0.0, 0.0, 0.0); 
		pass->setSelfIllumination(0.25, 0.75, 0.25);
		pass->setPointSize(5.0);
	}


	// Create manual objects and scene nodes for drawing bones and joints
	Ogre::SceneNode* mRoot = mSceneManager->getRootSceneNode();
	for(int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
	    for(int j = 0; j < SKELETON_BONE_COUNT; ++j)
		{
			Ogre::String indexName = Ogre::StringConverter::toString(i) + "_" + Ogre::StringConverter::toString(j);
			mSkeletonBone[i][j] = mSceneManager->createManualObject("Bone" + indexName);
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

		for(int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j)
		{
			Ogre::String indexName = Ogre::StringConverter::toString(i) + "_" + Ogre::StringConverter::toString(j);
			mSkeletonJoint[i][j] = mSceneManager->createManualObject("Joint" + indexName);
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




void KinectController::setupGPUSkeletonDraw(void)
{
	// Create render target textures for processing and drawing images from Kinect
	Ogre::TextureManager& textureManager = Ogre::TextureManager::getSingleton();
	//mColorTarget = textureManager.createManual("ColorTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mColorWidth, mColorHeight,
											 //  0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);
	mDepthTarget = textureManager.createManual("DepthTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, mDepthWidth, mDepthHeight,
											   0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);

	// Create cameras for processing images from Kinect
	//Ogre::Camera* colorCam = mSceneManager->createCamera("ColorCam");
	//colorCam->setAspectRatio(static_cast<Ogre::Real>(mColorWidth) / static_cast<Ogre::Real>(mColorHeight));

	//Ogre::Viewport* colorViewport = mColorTarget->getBuffer()->getRenderTarget()->addViewport(colorCam);
	//colorViewport->setClearEveryFrame(true);
	//colorViewport->setBackgroundColour(Ogre::ColourValue::Black);
	//colorViewport->setOverlaysEnabled(false);
		
	Ogre::Camera* depthCam = mSceneManager->createCamera("DepthCam");
	depthCam->setAspectRatio(static_cast<Ogre::Real>(mDepthWidth) / static_cast<Ogre::Real>(mDepthHeight));

	Ogre::Viewport* depthViewport = mDepthTarget->getBuffer()->getRenderTarget()->addViewport(depthCam);
	depthViewport->setClearEveryFrame(true);
	depthViewport->setBackgroundColour(Ogre::ColourValue::Black);
	depthViewport->setOverlaysEnabled(false);


	// Create materials for processing images from Kinect
	Ogre::MaterialManager& materialManager = Ogre::MaterialManager::getSingleton();
	//if(!materialManager.resourceExists("Kinect/Color"))
	//{
	//	Ogre::MaterialPtr material = materialManager.create("Kinect/Color", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//	Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
	//	pass->setSceneBlending(Ogre::SBT_REPLACE);
	//	pass->createTextureUnitState("ColorTexture");
	//}

	if(!materialManager.resourceExists("Kinect/Depth"))
	{
		Ogre::MaterialPtr material = materialManager.create("Kinect/Depth", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_REPLACE);
		pass->createTextureUnitState("DepthTexture");
	}


	// Create 2D planes for processing images from Kinect
	//Ogre::Rectangle2D* mColorRect = new Ogre::Rectangle2D(true);
	//mColorRect->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
	//mColorRect->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));
	//mColorRect->setMaterial("Kinect/Color");

	//mColorRectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("ColorRectNode");
	//mColorRectNode->attachObject(mColorRect);

	Ogre::Rectangle2D* mDepthRect = new Ogre::Rectangle2D(true);
	mDepthRect->setCorners(-.5f, .5f, .5f, -.5f);
	mDepthRect->setBoundingBox(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::Extent::EXTENT_INFINITE));
	mDepthRect->setMaterial("Kinect/Depth");

	mDepthRectNode = mSceneManager->getRootSceneNode()->createChildSceneNode("DepthRectNode");
	mDepthRectNode->attachObject(mDepthRect);
}