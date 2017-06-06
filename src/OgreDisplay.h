

#ifndef __OgreDisplay_h_
#define __OgreDisplay_h_

#include "btBulletDynamicsCommon.h"
#include "Ogre.h"

class OgreDisplay
{

public:
	OgreDisplay(btDynamicsWorld* _dynamicsWorld):dynamicsWorld(_dynamicsWorld)
	{
	}
	virtual ~OgreDisplay(void);

	void update();
	void createDynamicObject(Ogre::Entity*,btRigidBody*);
	void createStaticObject(Ogre::Entity*,btRigidBody*);
	btRigidBody* findBtRigidBodyFromEntity(Ogre::Entity* ent);
	void rotateRigidBody(Ogre::SceneNode* node,float angle);
	void rotateRigidBodyGroup(Ogre::SceneNode* node,float angle);
	Ogre::Entity* findEntityFromBtRigidBody( btRigidBody* body );

protected:
	std::vector<std::pair<Ogre::Entity*,btRigidBody*>> dynamicObjects;
	std::vector<std::pair<Ogre::Entity*,btRigidBody*>> staticObjects;
	btDynamicsWorld* dynamicsWorld;
};

#endif