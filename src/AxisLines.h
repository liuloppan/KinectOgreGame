


#ifndef __AxisLines_h_
#define __AxisLines_h_

#include "Ogre.h"
#include "DynamicLines.h"

/*
	This class has purpose to display joint orientation (three orthogonal axes)
	so it is easier to debug
*/
class AxisLines
{
public:
	AxisLines(void);
	virtual ~AxisLines(void);

	Ogre::Vector3 xAxis;
	Ogre::Vector3 yAxis;
	Ogre::Vector3 zAxis;

	Ogre::String color1;
	Ogre::String color2;
	Ogre::String color3;

	Ogre::Real length;

	void initAxis(Ogre::String boneName, Ogre::Entity* entity, Ogre::SceneManager* mSceneManager);
	void updateLines(Ogre::Vector3 xAxis, Ogre::Vector3 yAxis, Ogre::Vector3 zAxis);
	void setVisible(bool isXVisible, bool isYVisible, bool isZVisible);

protected:
	DynamicLines* xLine;
	DynamicLines* yLine;
	DynamicLines* zLine;

	bool isXVisible;
	bool isYVisible;
	bool isZVisible;
};

#endif