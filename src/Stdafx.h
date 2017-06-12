
// Kinect SDK Precompiled Header ============================================================================================

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>
#include <ole2.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#pragma comment ( lib, "winmm.lib" )
#pragma comment ( lib, "d2d1.lib" )

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

// OGRE 3D Precompiled Header ============================================================================================

#include "Ogre.h"
#include "OgreMath.h"
#include "OgreCamera.h"
#include "OgreEntity.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreSceneManager.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"

#include "OISEvents.h"
#include "OISInputManager.h"
#include "OISKeyboard.h"
#include "OISMouse.h"