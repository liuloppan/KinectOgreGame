
#include "Stdafx.h"
#include "AxisLines.h"

//-------------------------------------------------------------------------------------
AxisLines::AxisLines(void)
{
	this->color1 = "Line/Red";
	this->color2 = "Line/Green";
	this->color3 = "Line/Blue";

	this->length = 10;

	this->isXVisible = true;
	this->isYVisible = true;
	this->isZVisible = true;
}

//-------------------------------------------------------------------------------------
AxisLines::~AxisLines(void)
{	
}

//-------------------------------------------------------------------------------------
void AxisLines::setVisible(bool isXVisible, bool isYVisible, bool isZVisible)
{
	this->isXVisible = isXVisible;
	this->isYVisible = isYVisible;
	this->isZVisible = isZVisible;
}

//-------------------------------------------------------------------------------------
void AxisLines::initAxis(Ogre::String boneName, Ogre::Entity* entity, Ogre::SceneManager* mSceneManager)
{

	if(isXVisible)	/* red */
	{
		xLine = new DynamicLines(Ogre::RenderOperation::OT_LINE_LIST);
		entity->attachObjectToBone(boneName, xLine);
		xLine->setMaterial(color1);
	}

	if(isYVisible) /* green */
	{
		yLine = new DynamicLines(Ogre::RenderOperation::OT_LINE_LIST);
		entity->attachObjectToBone(boneName, yLine);
		yLine->setMaterial(color2);
	}

	if(isZVisible) /* blue */
	{
		zLine = new DynamicLines(Ogre::RenderOperation::OT_LINE_LIST);
		entity->attachObjectToBone(boneName, zLine);
		zLine->setMaterial(color3);
	}

	Ogre::Bone* bone = entity->getSkeleton()->getBone(boneName);
	Ogre::Quaternion q = bone->getOrientation();
	this->updateLines(q.xAxis(), q.yAxis(), q.zAxis());
}

//-------------------------------------------------------------------------------------
void AxisLines::updateLines(Ogre::Vector3 xAxis, Ogre::Vector3 yAxis, Ogre::Vector3 zAxis)
{
	if(isXVisible)
	{
		xLine->clear();
		xLine->addPoint(Ogre::Vector3::ZERO);
		xLine->addPoint(xAxis * length);
		xLine->update();
	}

	if(isYVisible)
	{
		yLine->clear();
		yLine->addPoint(Ogre::Vector3::ZERO);
		yLine->addPoint(yAxis * length);
		yLine->update();
	}

	if(isZVisible)
	{
		zLine->clear();
		zLine->addPoint(Ogre::Vector3::ZERO);
		zLine->addPoint(zAxis * length);
		zLine->update();
	}
}