#pragma once

#ifndef __UIManager_h_
#define __UIManager_h_

#include "Stdafx.h"
#include "SdkTrays.h"


class UIManager {

public:
	
	UIManager(OgreBites::SdkTrayManager *aTrayMgr);
	virtual ~UIManager();

	void setupWidgets();

	OgreBites::SdkTrayManager *mTrayMgr;

};

#endif