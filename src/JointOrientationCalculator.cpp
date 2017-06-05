
#include "Stdafx.h"
#include "JointOrientationCalculator.h"

//-------------------------------------------------------------------------------------
JointOrientationCalculator::JointOrientationCalculator(void) :
	isMirror(false)
{

}

//-------------------------------------------------------------------------------------
JointOrientationCalculator::~JointOrientationCalculator(void)
{

}

//-------------------------------------------------------------------------------------
void JointOrientationCalculator::setupController(KinectController* controller)
{
	this->controller = controller;
}

//-------------------------------------------------------------------------------------
// check whether the two vectors lie in the same line (and also parallel)
bool JointOrientationCalculator::areNearCollinear(Ogre::Vector3 v1, Ogre::Vector3 v2)
{
	v1.normalise();
	v2.normalise();

	Ogre::Real product = v1.dotProduct(v2);

	return product >= 0.8f;		// [0, 1]
}

//-------------------------------------------------------------------------------------
Ogre::Vector3 JointOrientationCalculator::getDirection(NuiManager::NuiJointIndex p1, NuiManager::NuiJointIndex p2)
{
	Ogre::Vector3 pVec1 = controller->getJointPosition(p1);
	Ogre::Vector3 pVec2 = controller->getJointPosition(p2);

	return pVec2 - pVec1;
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::buildQuaternion(Ogre::Vector3 xAxis, Ogre::Vector3 yAxis, Ogre::Vector3 zAxis)
{

	Ogre::Matrix3 mat;

	if(isMirror)
	{
		mat = Ogre::Matrix3(xAxis.x, yAxis.x, zAxis.x,
						    xAxis.y, yAxis.y, zAxis.y,
						    xAxis.z, yAxis.z, zAxis.z);

		Ogre::Matrix3 flipMat(1, 0,  0,
							  0, 1,  0,
							  0, 0, -1);

		mat = flipMat * mat * flipMat;
	}
	else
	{

		mat = Ogre::Matrix3(xAxis.x, -yAxis.x, zAxis.x,
						   -xAxis.y, yAxis.y, -zAxis.y,
						   xAxis.z,  -yAxis.z, zAxis.z);
	}

	Ogre::Quaternion q;
	q.FromRotationMatrix(mat);
	
	

	return q;
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromX(Ogre::Vector3 v1)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;
	
	xCol = v1.normalisedCopy();

	yCol.x = 0.0f;
	yCol.y = xCol.z;
	yCol.z = -xCol.y;

	yCol.normalise();

	zCol = xCol.crossProduct(yCol);

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromY(Ogre::Vector3 v1)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;

	yCol = v1.normalisedCopy();

	xCol.x = yCol.y;
	xCol.y = -yCol.x;
	xCol.z = 0.0f;

	xCol.normalise();

	zCol = xCol.crossProduct(yCol);		// do not normalize

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromZ(Ogre::Vector3 v1)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;

	zCol = v1.normalisedCopy();

	xCol.x = zCol.y;
	xCol.y = -zCol.x;
	xCol.z = 0.0f;

	xCol.normalise();

	yCol = zCol.crossProduct(xCol);		// do not normalize

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromXY(Ogre::Vector3 xUnnormalized, Ogre::Vector3 yUnnormalized)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;

	Ogre::Vector3 xn = xUnnormalized.normalisedCopy();
	Ogre::Vector3 yn = yUnnormalized.normalisedCopy();

	xCol = xn;
	zCol = xCol.crossProduct(yn);
	yCol = zCol.crossProduct(xCol);

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromYX(Ogre::Vector3 xUnnormalized, Ogre::Vector3 yUnnormalized)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;
	
	Ogre::Vector3 xn = xUnnormalized.normalisedCopy();
	Ogre::Vector3 yn = yUnnormalized.normalisedCopy();

	yCol = yn;
	zCol = xn.crossProduct(yCol);
	xCol = yCol.crossProduct(zCol);

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::makeOrientationFromYZ(Ogre::Vector3 yUnnormalized, Ogre::Vector3 zUnnormalized)
{
	// matrix column
	Ogre::Vector3 xCol;
	Ogre::Vector3 yCol;
	Ogre::Vector3 zCol;

	Ogre::Vector3 yn = yUnnormalized.normalisedCopy();
	Ogre::Vector3 zn = zUnnormalized.normalisedCopy();

	yCol = yn;
	xCol = yCol.crossProduct(zn);
	zCol = xCol.crossProduct(yCol);

	return this->buildQuaternion(xCol, yCol, zCol);
}

//-------------------------------------------------------------------------------------
Ogre::Quaternion JointOrientationCalculator::getSkeletonJointOrientation(NuiManager::NuiJointIndex idx)
{
	Ogre::Quaternion orientation;
	
	Ogre::Vector3 vx = Ogre::Vector3::UNIT_X;
	Ogre::Vector3 vy = Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 vz = Ogre::Vector3::UNIT_Z;

	switch(idx)
	{
	case NuiJointIndex::HIP_CENTER:			/*0*/
		{
			vx = this->getDirection(NuiJointIndex::HIP_LEFT,		NuiJointIndex::HIP_RIGHT);

			/* vy should be calculated by using HIP_CENTER and SPINE but the angle between them
				is often too wide so the skinned model will have weird hip orientation */
			//vy = this->getDirection(NuiJointIndex::HIP_CENTER,	NuiJointIndex::SPINE);
			vy = Ogre::Vector3::UNIT_Y;		

			orientation = this->makeOrientationFromYX(vx, vy);
		}
		break;

	case NuiJointIndex::SPINE:				/*1*/
		{
			vx = this->getDirection(NuiJointIndex::SHOULDER_LEFT,	NuiJointIndex::SHOULDER_RIGHT);
			vy = this->getDirection(NuiJointIndex::SPINE,			NuiJointIndex::SHOULDER_CENTER);
			
			orientation = this->makeOrientationFromYX(vx, vy);
		}
		break;

	case NuiJointIndex::SHOULDER_CENTER:	/*2*/
		{
			vx = this->getDirection(NuiJointIndex::SHOULDER_LEFT,	NuiJointIndex::SHOULDER_RIGHT);
			vy = this->getDirection(NuiJointIndex::SHOULDER_CENTER, NuiJointIndex::HEAD);			
			orientation = this->makeOrientationFromYX(vx, vy);
		}
		break;

	case NuiJointIndex::HEAD:				/*3*/
		{
			vy = this->getDirection(NuiJointIndex::SHOULDER_CENTER, NuiJointIndex::HEAD);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::SHOULDER_LEFT:		/*4*/
		{
			vx = this->getDirection(NuiJointIndex::SHOULDER_LEFT,	NuiJointIndex::SHOULDER_RIGHT);
			vy = this->getDirection(NuiJointIndex::ELBOW_LEFT,		NuiJointIndex::SHOULDER_LEFT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::SHOULDER_RIGHT:		/*8*/
		{
			vx = this->getDirection(NuiJointIndex::SHOULDER_LEFT,	NuiJointIndex::SHOULDER_RIGHT);
			vy = this->getDirection(NuiJointIndex::ELBOW_RIGHT,		NuiJointIndex::SHOULDER_RIGHT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::ELBOW_LEFT:			/*5*/
		{
			vy = this->getDirection(NuiJointIndex::WRIST_LEFT,		NuiJointIndex::ELBOW_LEFT);
			//vx = Ogre::Vector3::UNIT_X;
			//orientation = this->makeOrientationFromYX(vx, vy);
			vz = Ogre::Vector3::UNIT_Z;
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::ELBOW_RIGHT:		/*9*/
		{
			vy = this->getDirection(NuiJointIndex::WRIST_RIGHT,		NuiJointIndex::ELBOW_RIGHT);
			//vx = Ogre::Vector3::UNIT_X;
			//orientation = this->makeOrientationFromYX(vx, vy);
			vz = Ogre::Vector3::UNIT_Z;
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::WRIST_LEFT:			/*6*/
		{
			vz = Ogre::Vector3::UNIT_Z;
			vy = this->getDirection(NuiJointIndex::WRIST_LEFT,		NuiJointIndex::ELBOW_LEFT);		
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::WRIST_RIGHT:		/*10*/
		{
			vz = Ogre::Vector3::UNIT_Z;
			vy = this->getDirection(NuiJointIndex::WRIST_RIGHT,		NuiJointIndex::ELBOW_RIGHT);
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::HAND_LEFT:			/*7*/
		{
			vy = this->getDirection(NuiJointIndex::HAND_LEFT,		NuiJointIndex::WRIST_LEFT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::HAND_RIGHT:			/*11*/
		{
			vy = this->getDirection(NuiJointIndex::HAND_RIGHT,		NuiJointIndex::WRIST_RIGHT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::HIP_LEFT:			/*8*/
		{
			vy = this->getDirection(NuiJointIndex::KNEE_LEFT,		NuiJointIndex::HIP_LEFT);
			vx = this->getDirection(NuiJointIndex::HIP_LEFT,		NuiJointIndex::HIP_RIGHT);
			orientation = this->makeOrientationFromYX(vx, vy);
		}
		break;

	case NuiJointIndex::HIP_RIGHT:			/*16*/
		{
			vy = this->getDirection(NuiJointIndex::KNEE_RIGHT,		NuiJointIndex::HIP_RIGHT);
			vx = this->getDirection(NuiJointIndex::HIP_LEFT,		NuiJointIndex::HIP_RIGHT);
			orientation = this->makeOrientationFromYX(vx, vy);
		}
		break;

	case NuiJointIndex::KNEE_LEFT:			/*13*/
		{
			vy = this->getDirection(NuiJointIndex::ANKLE_LEFT,		NuiJointIndex::KNEE_LEFT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::KNEE_RIGHT:			/*17*/
		{
			vy = this->getDirection(NuiJointIndex::ANKLE_RIGHT,		NuiJointIndex::KNEE_RIGHT);
			orientation = this->makeOrientationFromY(vy);
		}
		break;

	case NuiJointIndex::ANKLE_LEFT:			/*14*/
		{
			vy = this->getDirection(NuiJointIndex::ANKLE_LEFT,		NuiJointIndex::KNEE_LEFT);
			vz = this->getDirection(NuiJointIndex::ANKLE_LEFT,		NuiJointIndex::FOOT_LEFT);
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::ANKLE_RIGHT:		/*18*/
		{
			vy = this->getDirection(NuiJointIndex::ANKLE_RIGHT,		NuiJointIndex::KNEE_RIGHT);
			vz = this->getDirection(NuiJointIndex::ANKLE_RIGHT,		NuiJointIndex::FOOT_RIGHT);
			orientation = this->makeOrientationFromYZ(vy, vz);
		}
		break;

	case NuiJointIndex::FOOT_LEFT:			/*15*/
		{
			vz = this->getDirection(NuiJointIndex::ANKLE_LEFT,		NuiJointIndex::FOOT_LEFT);
			orientation = this->makeOrientationFromZ(vz);
		}
		break;
	
	case NuiJointIndex::FOOT_RIGHT:			/*19*/
		{
			vz = this->getDirection(NuiJointIndex::ANKLE_RIGHT,		NuiJointIndex::FOOT_RIGHT);
			orientation = this->makeOrientationFromZ(vz);
		}
		break;

	}

	return orientation;
}

//-------------------------------------------------------------------------------------
void JointOrientationCalculator::setMirror(bool isMirror)
{
	this->isMirror = isMirror;
}

//-------------------------------------------------------------------------------------
bool JointOrientationCalculator::getMirror(void)
{
	return isMirror;
}