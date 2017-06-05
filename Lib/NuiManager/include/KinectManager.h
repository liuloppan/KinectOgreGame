
#ifndef __KinectSkeleton_h__
#define __KinectSkeleton_h__

#include "NuiApi.h"
#include "NuiDataType.h"

namespace NuiManager 
{
	/*
		Wrapper class to bridge Kinect API and other C++ program
	*/
	class KinectManager
	{
	public:
		KinectManager(void);
		virtual ~KinectManager(void);

		HRESULT InitNui(void);
		void UnInitNui(void);

		size_t getDeviceCount(void);

		BYTE* getColorBuffer();
		BYTE* getDepthBuffer();
		NUI_SKELETON_DATA* getSkeleton(int index = 0);

	protected:
		INuiSensor*         nuiSensor;
		BSTR                instanceId;

		BYTE*				depthBuffer;
		BYTE*				colorBuffer;
		NUI_SKELETON_DATA*	nuiSkeletonData;

	protected:
		bool trackColorImage(void);
		bool trackDepthImage(void);
		bool trackSkeleton(void);

	private:
		DWORD WINAPI            nuiProcessThread();
		static DWORD WINAPI     nuiProcessThread(LPVOID pParam);

	private:
		HANDLE        hThNuiProcess;
		HANDLE        hEvNuiProcessStop;

		HANDLE        hNextDepthFrameEvent;
		HANDLE        hNextColorFrameEvent;
		HANDLE        hNextSkeletonEvent;
		HANDLE        pDepthStreamHandle;
		HANDLE        pVideoStreamHandle;

		DWORD         lastDepthFPStime;
		DWORD         lastSkeletonFoundTime;

		int           depthFramesTotal;
		int           lastDepthFramesTotal;
		int			  frameRate;
	};
}

#endif