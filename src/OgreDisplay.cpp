
#include "Stdafx.h"
#include "OgreDisplay.h"

//-------------------------------------------------------------------------------------
void OgreDisplay::update()
{	
	/*
	for(int i = 0; i < dynamicObjects.size(); i++)
	{
		try
		{
			btTransform tr;
			dynamicObjects[i].second->getMotionState()->getWorldTransform(tr);
			btVector3 pos = tr.getOrigin();
			btQuaternion qut = tr.getRotation();
			dynamicObjects[i].first->getParentSceneNode()->setOrientation(Ogre::Quaternion(qut.getW(), qut.getX(), qut.getY(), qut.getZ()));
			Ogre::Vector3 localPos = dynamicObjects[i].first->getParentSceneNode()->getParentSceneNode()->convertWorldToLocalPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z()));
			dynamicObjects[i].first->getParentSceneNode()->setPosition(localPos);
		}
		catch(Ogre::Exception e)
		{
		}
	}
	*/
}

//-------------------------------------------------------------------------------------
void OgreDisplay::createDynamicObject(Ogre::Entity* OgreObj, btRigidBody* BulletObj)
{
	dynamicsWorld->addRigidBody(BulletObj);
	std::pair<Ogre::Entity*, btRigidBody*> obj;
	obj.first = OgreObj;
	obj.second = BulletObj;
	dynamicObjects.push_back(obj);
}

//-------------------------------------------------------------------------------------
void OgreDisplay::createStaticObject( Ogre::Entity* OgreObj,btRigidBody* BulletObj)
{
	dynamicsWorld->addRigidBody(BulletObj);
	std::pair<Ogre::Entity*, btRigidBody*> obj;
	obj.first = OgreObj;
	obj.second = BulletObj;
	staticObjects.push_back(obj);
}

//-------------------------------------------------------------------------------------
OgreDisplay::~OgreDisplay( void )
{
}

//-------------------------------------------------------------------------------------
btRigidBody* OgreDisplay::findBtRigidBodyFromEntity( Ogre::Entity* ent )
{
	for(unsigned short i = 0; i < staticObjects.size(); ++i)
	{
		if(staticObjects[i].first==ent) 
			return staticObjects[i].second;
	}

	for(unsigned short i = 0; i < dynamicObjects.size(); ++i)
	{
		if(dynamicObjects[i].first == ent) 
			return dynamicObjects[i].second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
Ogre::Entity* OgreDisplay::findEntityFromBtRigidBody( btRigidBody* body )
{
	for(unsigned short i = 0; i < staticObjects.size(); ++i)
	{
		if(staticObjects[i].second == body) 
			return staticObjects[i].first;
	}

	for(unsigned short i = 0; i < dynamicObjects.size(); ++i)
	{
		if(dynamicObjects[i].second == body) 
			return dynamicObjects[i].first;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void OgreDisplay::rotateRigidBody(Ogre::SceneNode* node,float angle)
{
	if(node->getName() == "VehicleNodeMain") { return; }

	Ogre::Entity* ent = NULL;

	try
	{
		ent = dynamic_cast<Ogre::Entity*>(node->getAttachedObject(node->getName()));
	}
	catch(Ogre::Exception e)
	{
		return;
	}

	btRigidBody* btRotateObj = findBtRigidBodyFromEntity(ent);

	if(btRotateObj)
	{
		btTransform tns;
		btRotateObj->getMotionState()->getWorldTransform(tns);
		btVector3 origin(tns.getOrigin());
		tns.setOrigin(btVector3(0, 0, 0));
		btQuaternion originalRotation (tns.getRotation());
		Ogre::Quaternion rotToAngle(Ogre::Degree(angle), Ogre::Vector3::UNIT_Y);
		btQuaternion btRotToAngle(rotToAngle.x, rotToAngle.y, rotToAngle.z, rotToAngle.w);
		tns.setRotation(originalRotation * btRotToAngle);
		tns.setOrigin(origin);
		btRotateObj->getMotionState()->setWorldTransform(tns);
		btRotateObj->getMotionState()->getWorldTransform(tns);

	}
}

//-------------------------------------------------------------------------------------
void OgreDisplay::rotateRigidBodyGroup(Ogre::SceneNode* node, float angle)
{
	for(int i = 0; i < node->numChildren(); i++)
	{
		Ogre::Entity* ent = dynamic_cast<Ogre::Entity*>(dynamic_cast<Ogre::SceneNode*>(node->getChild(i))->getAttachedObject(0));
		btRigidBody* btRotateObj = findBtRigidBodyFromEntity(ent);
		if(btRotateObj)
		{
			btTransform tr;
			btRotateObj->getMotionState()->getWorldTransform(tr);
			Ogre::Vector3 localPos = ent->getParentSceneNode()->getPosition();
			Ogre::Vector3 worldPos = ent->getParentSceneNode()->getParentSceneNode()->convertLocalToWorldPosition(localPos);
			Ogre::Quaternion rot = ent->getParentSceneNode()->getOrientation();
			tr.setOrigin(btVector3(worldPos.x, worldPos.y, worldPos.z));
			tr.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
			btRotateObj->getMotionState()->setWorldTransform(tr);
		}
	}
}

