
#include "Stdafx.h"
#include "KinectManager.h"
#include <mmsystem.h>
#include <assert.h>
#include <strsafe.h>

//-------------------------------------------------------------------------------------
NuiManager::KinectManager::KinectManager(void)
	:nuiSensor(0),
	lastDepthFPStime(0),
	nuiSkeletonData(0)
{
	colorBuffer = (BYTE*) malloc (640 * 480 * 4 * sizeof(BYTE));
	depthBuffer = (BYTE*) malloc (640 * 480 * 4 * sizeof(BYTE));
}

//-------------------------------------------------------------------------------------
NuiManager::KinectManager::~KinectManager(void)
{
}

//-------------------------------------------------------------------------------------
HRESULT NuiManager::KinectManager::InitNui(void)
{	
	HRESULT hr;

	nuiSkeletonData = (NUI_SKELETON_DATA*)malloc(NUI_SKELETON_COUNT * sizeof (NUI_SKELETON_DATA));

	if(!nuiSensor)
	{
		hr = NuiCreateSensorByIndex(0, &nuiSensor);
		if(FAILED(hr)) return hr;
		
		instanceId = nuiSensor->NuiDeviceConnectionId();
	}

	hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// Initialize Nui
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
	hr = nuiSensor->NuiInitialize( nuiFlags );

	if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
	{
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
		hr = nuiSensor->NuiInitialize( nuiFlags) ;
	}

	// Skeleton Tracking
	if(FAILED(hr)) return hr;

	if ( HasSkeletalEngine( nuiSensor ) )
	{
		hr = nuiSensor->NuiSkeletonTrackingEnable( hNextSkeletonEvent, 0 );
		if(FAILED(hr)) return hr;
	}

	// RGB Image
	hr = nuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		hNextColorFrameEvent,
		&pVideoStreamHandle );

	if(FAILED(hr)) return hr;
		
	// Depth Image
	hr = nuiSensor->NuiImageStreamOpen(
		HasSkeletalEngine(nuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		hNextDepthFrameEvent,
		&pDepthStreamHandle );

	if(FAILED(hr)) return hr;
	
	// Start the Nui processing thread
	hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	hThNuiProcess = CreateThread( NULL, 0, nuiProcessThread, this, 0, NULL );

	
		
	return hr;
}

//-------------------------------------------------------------------------------------
void NuiManager::KinectManager::UnInitNui(void)
{
	// delete data
	//if(nuiColorFrame)		delete nuiColorFrame;
	//if(nuiDepthFrame)		delete nuiDepthFrame;
	//if(nuiSkeletonFrame)	delete nuiSkeletonFrame;
	if(nuiSkeletonData)		delete nuiSkeletonData;

	// stop the Nui processing thread
	if ( hEvNuiProcessStop != NULL )
	{
		// Signal the thread
		SetEvent(hEvNuiProcessStop);

		// Wait for thread to stop
		if ( hThNuiProcess != NULL )
		{
			WaitForSingleObject( hThNuiProcess, INFINITE );
			CloseHandle( hThNuiProcess );
		}
		CloseHandle( hEvNuiProcessStop );
	}

	if (nuiSensor) { nuiSensor->NuiShutdown(); }

	if (hNextSkeletonEvent && (hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(hNextSkeletonEvent);
		hNextSkeletonEvent = NULL;
	}

	if (hNextDepthFrameEvent && (hNextDepthFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(hNextDepthFrameEvent);
		hNextDepthFrameEvent = NULL;
	}

	if (hNextColorFrameEvent && (hNextColorFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(hNextColorFrameEvent);
		hNextColorFrameEvent = NULL;
	}

	if (nuiSensor)
	{
		nuiSensor->Release();
		nuiSensor = NULL;
	}

	// delete buffers
	free(depthBuffer);
	free(colorBuffer);
}


//-------------------------------------------------------------------------------------
size_t NuiManager::KinectManager::getDeviceCount(void)
{
	int result(0); 
	NuiGetSensorCount(&result);
	
	return (size_t) result;
}

//-------------------------------------------------------------------------------------
bool NuiManager::KinectManager::trackSkeleton(void)
{
	NUI_SKELETON_FRAME skeletonFrame = {0};
	bool foundSkeleton = false;

	if(SUCCEEDED(nuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame)))
	{
		for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
		{
			if( skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
				(skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY))
			{
				foundSkeleton = true;
			}
		}
	}

	// no skeletons!
	if( !foundSkeleton ) { return false; }

	// smooth out the skeleton data
	HRESULT hr = nuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);
	if ( FAILED(hr) ) { return false; }

	//NUI_SKELETON_FRAME temp = skeletonFrame;
	//nuiSkeletonFrame = &temp;

	for(int i = 0; i < NUI_SKELETON_COUNT; i++)
		nuiSkeletonData[i] = skeletonFrame.SkeletonData[i];

	lastSkeletonFoundTime = timeGetTime();

	return true;
}

//-------------------------------------------------------------------------------------
bool NuiManager::KinectManager::trackColorImage(void)
{
	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = nuiSensor->NuiImageStreamGetNextFrame(pVideoStreamHandle, 0, &imageFrame);
	if(FAILED(hr)) return false;

	INuiFrameTexture* texture = imageFrame.pFrameTexture;

	NUI_LOCKED_RECT lockedRect;
	texture->LockRect(0, &lockedRect, NULL, 0);

	if(lockedRect.Pitch != 0)
	{				
		BYTE* pBits = (BYTE*) lockedRect.pBits;
		int offset = 0;

		for(size_t i = 0; i < 640; i++)
		{
			for (size_t j = 0; j < 480; j++)
			{
				colorBuffer[offset++] = pBits[0]; // B
				colorBuffer[offset++] = pBits[1]; // G
				colorBuffer[offset++] = pBits[2]; // R
				colorBuffer[offset++] = 254;      // A

				pBits += 4;
			}
		}
	}

	texture->UnlockRect(0);

	nuiSensor->NuiImageStreamReleaseFrame(pVideoStreamHandle, &imageFrame);
	
	return true;
}

//-------------------------------------------------------------------------------------
bool NuiManager::KinectManager::trackDepthImage(void)
{
	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = nuiSensor->NuiImageStreamGetNextFrame(pDepthStreamHandle, 0, &imageFrame);
	if(FAILED(hr)) return false;

	INuiFrameTexture* texture = imageFrame.pFrameTexture;

	NUI_LOCKED_RECT lockedRect;
	texture->LockRect(0, &lockedRect, NULL, 0);

	if(lockedRect.Pitch != 0)
	{
		BYTE* pBits = (BYTE*) lockedRect.pBits;
		/*int offset = 0;

		for(size_t i = 0; i < 320; i++)
		{
			for (size_t j = 0; j < 240; j++)
			{
				depthBuffer[offset++] = pBits[0]; // B
				depthBuffer[offset++] = pBits[1]; // G
				depthBuffer[offset++] = pBits[2]; // R
				depthBuffer[offset++] = 254;	  // A

				pBits += 4;
			}
		}*/
	}

	texture->UnlockRect(0);

	nuiSensor->NuiImageStreamReleaseFrame(pDepthStreamHandle, &imageFrame);
	
	return true;
}

//-------------------------------------------------------------------------------------
DWORD WINAPI NuiManager::KinectManager::nuiProcessThread(LPVOID pParam)
{
	KinectManager *manager = (KinectManager *) pParam;
	return manager->nuiProcessThread();
}

//-------------------------------------------------------------------------------------
// nuiProcessThread
// Thread to handle Kinect processing
//-------------------------------------------------------------------------------------
DWORD WINAPI NuiManager::KinectManager::nuiProcessThread()
{
	const int numEvents = 4;
	HANDLE hEvents[numEvents] = { hEvNuiProcessStop, hNextDepthFrameEvent, hNextColorFrameEvent, hNextSkeletonEvent };
	int    nEventIdx;
	DWORD  t;

	lastDepthFPStime = timeGetTime();
	lastSkeletonFoundTime = 0;
	//frameRate = 0;
	
	// thread loop
	bool continueProcessing = true;
	while(continueProcessing)
	{
		// wait for any of the events to be signaled
		nEventIdx = WaitForMultipleObjects(numEvents, hEvents, FALSE, 100);

		// process signal events
		if(nEventIdx == WAIT_TIMEOUT)
		{
			continue;
		}
		else if(nEventIdx == WAIT_OBJECT_0)
		{
			continueProcessing = false;
			continue;
		}
		else if(nEventIdx == (WAIT_OBJECT_0 + 1))
		{
			trackDepthImage();			
			++depthFramesTotal;
		}
		else if(nEventIdx == (WAIT_OBJECT_0 + 2))
		{
			trackColorImage();
		}
		else if(nEventIdx == (WAIT_OBJECT_0 + 3))
		{
			trackSkeleton();
		}

		t = timeGetTime();
		if((t - lastDepthFPStime) > 1000)
		{
			frameRate = ((depthFramesTotal - lastDepthFramesTotal) * 1000 + 500) / (t - lastDepthFPStime);
			lastDepthFramesTotal = depthFramesTotal;
			lastDepthFPStime = t;
		}

	}

	return 0;
}

//-------------------------------------------------------------------------------------
NUI_SKELETON_DATA* NuiManager::KinectManager::getSkeleton(int playerIndex)
{
	return &nuiSkeletonData[playerIndex];
}

//-------------------------------------------------------------------------------------
BYTE* NuiManager::KinectManager::getColorBuffer()
{
	return colorBuffer;
}

//-------------------------------------------------------------------------------------
BYTE* NuiManager::KinectManager::getDepthBuffer()
{
	return depthBuffer;
}