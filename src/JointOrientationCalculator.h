
#ifndef __JointOrientationCalculator_h__
#define __JointOrientationCalculator_h__

#include "Stdafx.h"
#include "NuiApi.h"
#include "NuiDataType.h"
#include "KinectController.h"

/*
	Class that is responsible to "guess" every joint orientation

		+X to point to the right of monitor
		+Y to point to the top of monitor
		+Z to point out of monitor

	based on:
		kinect-mssdk-openni-bridge
		https://www.assembla.com/code/kinect-mssdk-openni-bridge/git/nodes/master/src/JointOrientationCalculator.h
*/
class JointOrientationCalculator
{
public:
	JointOrientationCalculator(void);
	virtual ~JointOrientationCalculator(void);

	void setupController(KinectController* controller);
	Ogre::Quaternion getSkeletonJointOrientation(NuiManager::NuiJointIndex idx);

	void setMirror(bool isMirror);
	bool getMirror(void);
		
protected:
	KinectController* controller;
	bool isMirror;

protected:
	bool areNearCollinear(Ogre::Vector3 v1, Ogre::Vector3 v2);
	Ogre::Vector3 getDirection(NuiManager::NuiJointIndex p1, NuiManager::NuiJointIndex p2);

	Ogre::Quaternion buildQuaternion(Ogre::Vector3 xAxis, Ogre::Vector3 yAxis, Ogre::Vector3 zAxis);

	Ogre::Quaternion makeOrientationFromX(Ogre::Vector3 v1);
	Ogre::Quaternion makeOrientationFromY(Ogre::Vector3 v1);
	Ogre::Quaternion makeOrientationFromZ(Ogre::Vector3 v1);

	Ogre::Quaternion makeOrientationFromXY(Ogre::Vector3 xUnnormalized, Ogre::Vector3 yUnnormalized);
	Ogre::Quaternion makeOrientationFromYX(Ogre::Vector3 xUnnormalized, Ogre::Vector3 yUnnormalized);
	Ogre::Quaternion makeOrientationFromYZ(Ogre::Vector3 yUnnormalized, Ogre::Vector3 zUnnormalized);

};


#endif