#include "Stdafx.h"
#include "UIManager.h"
using namespace OgreBites;

UIManager::UIManager(OgreBites::SdkTrayManager *aTrayMgr)
{
	mTrayMgr = aTrayMgr;
	
	// Load fonts for tray captions
    //FontManager::getSingleton().getByName("SdkTrays/Caption")->load();
	setupWidgets();

}

UIManager::~UIManager()
{
}

void UIManager::setupWidgets()
{
	//mTrayMgr->destroyAllWidgets();
    // create check boxes to toggle the visibility of our particle systems
    const int WIDTH_UI = 160;
    // main menu
	if(mTrayMgr){
		mTrayMgr->createLabel(TL_CENTER, "mMainMenuLabel", "Main Menu", WIDTH_UI);
		mTrayMgr->createButton(TL_CENTER, "mOptionButton", "Option");
		mTrayMgr->createButton(TL_CENTER, "mCreditButton", "About");
		mTrayMgr->createButton(TL_CENTER, "mQuitButton", "Quit");
	}
 //  // mTrayMgr->hideAll();

	//mTrayMgr->moveWidgetToTray("mMainMenuLabel", TL_CENTER);
 //   mTrayMgr->moveWidgetToTray("mOptionButton", TL_CENTER);
 //   mTrayMgr->moveWidgetToTray("mCreditButton", TL_CENTER);
 //   mTrayMgr->moveWidgetToTray("mQuitButton", TL_CENTER);
 //   mTrayMgr->getWidget("mMainMenuLabel")->show();
 //   mTrayMgr->getWidget("mOptionButton")->show();
 //   mTrayMgr->getWidget("mCreditButton")->show();
 //   mTrayMgr->getWidget("mQuitButton")->show();
}