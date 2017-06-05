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

using namespace Ogre;
using namespace OgreBites;

class OgreKinectGame : public BaseApplication
{
public:
    OgreKinectGame();
    virtual ~OgreKinectGame();

protected:
    virtual void createScene();
    virtual void destroyScene();
    virtual bool frameRenderingQueued(const FrameEvent &fe);
    virtual bool keyPressed(const OIS::KeyEvent &evt);
    virtual bool keyReleased(const OIS::KeyEvent &evt);
    virtual bool mouseMoved(const OIS::MouseEvent &evt);
    virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
    virtual void buttonHit(Button *b);

    // setting up the scene
    virtual void checkBoxToggled(CheckBox *box);
    virtual void itemSelected(SelectMenu *menu);
    virtual bool setup();
    virtual void setupWidgets();
private:
    void setupToggles();
    void setMenuVisible(const String &name, bool visible = true);
	void setupParticles();

    bool mMenuName;
    Entity *mFloor;
    Light *mLight;
    SinbadCharacterController *mChara;
    Ogre::NameValuePairList mInfo;
};

#endif // #ifndef __OgreKinectGame_h_