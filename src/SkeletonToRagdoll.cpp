
#include "Stdafx.h"
#include "SkeletonToRagdoll.h"

//-------------------------------------------------------------------------------------
SkeletonToRagdoll::~SkeletonToRagdoll(void)
{	
}

//-------------------------------------------------------------------------------------
void SkeletonToRagdoll::setDebugBones(bool showDebugBones)
{
	this->showDebugBones = showDebugBones;
	for(int i = 0; i < debugBones.size(); i++)
		debugBones[i]->setVisible(this->showDebugBones);
}

//-------------------------------------------------------------------------------------
void SkeletonToRagdoll::createRagdoll( btDynamicsWorld* dynamicsWorld, Ogre::SceneNode* skeletonNode )
{
	steps = 0;
	this->dynamicsWorld = dynamicsWorld;
	scale = skeletonNode->getScale();
	this->skeletonNode = skeletonNode;

	skeleton = dynamic_cast<Ogre::Entity*>(skeletonNode->getAttachedObject(0))->getSkeleton();
	btVector3 positionOffset(skeletonNode->_getDerivedPosition().x, skeletonNode->_getDerivedPosition().y, skeletonNode->_getDerivedPosition().z);

	setBone(skeleton->getRootBone());
}

//-------------------------------------------------------------------------------------
btRigidBody* SkeletonToRagdoll::localCreateRigidBody (btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
{
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) shape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	dynamicsWorld->addRigidBody(body);

	return body;
}

//-------------------------------------------------------------------------------------
std::string SkeletonToRagdoll::update()
{
	resetAfterOffset();
	
	for(int i = 0; i < skeleton->getNumBones(); i++)
	{
		

		if(i > 0)
		{
			
			Ogre::Vector3 parentBonePos = skeletonNode->_getDerivedOrientation() * btBones[i].second->getParent()->_getDerivedPosition() * scale+skeletonNode->_getDerivedPosition();
			Ogre::Vector3 boneOrient = bonePos - parentBonePos;
			Ogre::Vector3 boneFinalPos = parentBonePos.midPoint(bonePos);
			Ogre::Quaternion boneFinalRotation = boneOrient.normalisedCopy().getRotationTo(Ogre::Vector3(0, 0, -1)).Inverse();

			debugBones[i]->_setDerivedPosition(boneFinalPos);
			debugBones[i]->_setDerivedOrientation(boneFinalRotation);
			tr.setOrigin(btVector3(boneFinalPos.x, boneFinalPos.y, boneFinalPos.z));
			tr.setRotation(btQuaternion(boneFinalRotation.getYaw().valueRadians(),boneFinalRotation.getPitch().valueRadians(), boneFinalRotation.getRoll().valueRadians()));
		}
		else
		{
			debugBones[i]->_setDerivedPosition(bonePos);
			tr.setOrigin(btVector3(bonePos.x,bonePos.y,bonePos.z));
		}
		
		btBones[i].first->setWorldTransform(tr);
	}
	
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();

	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btRigidBody* AB = static_cast<btRigidBody*>(contactManifold->getBody0());
		btRigidBody* BB = static_cast<btRigidBody*>(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j<numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);

			if (pt.getDistance() < 0.f)
			{
				for(int j = 0; j<btBones.size(); j++)
				{
					if(AB == btBones[j].first)
					{
						
						bool found =false;
						for(int k = 0; k < btBones.size(); k++)
						{
							if(BB == btBones[k].first)
							{
								found = true;
								break;
							}
						}

						for(int k = 0; k < ignoreEventObjs.size(); k++)
						{
							if(BB == ignoreEventObjs[k])
							{
								found = true;
								break;
							}
						}

						if(!found) 
						{
							for(int k = 0; k < bonesToReset.size(); k++)
							{
								if(btBones[j].second == bonesToReset[k].first)
								{
									found = true;
									break;
								}
							}

							if(!found)
							{
								
								btBones[j].second->setManuallyControlled(true);
								std::pair<Ogre::Bone*,Ogre::Vector3> tmp;
								tmp.first = btBones[j].second;
								btBones[j].second->translate(-0.1 * Ogre::Vector3(BB->getDeltaAngularVelocity().getX(),BB->getDeltaAngularVelocity().getY(),BB->getDeltaAngularVelocity().getZ()));
								tmp.second = btBones[j].second->getPosition();
								btBones[j].second->setManuallyControlled(false);
								bonesToReset.push_back(tmp);
								return btBones[j].second->getName();
							}
							
						}
					}

					if(BB == btBones[j].first)
					{
						bool found = false;
						for(int k = 0;k<btBones.size();k++)
						{
							if(AB == btBones[k].first)
							{
								found = true;
								break;
							}
						}

						for(int k = 0;k < ignoreEventObjs.size(); k++)
						{
							if(AB == ignoreEventObjs[k])
							{
								found = true;
								break;
							}
						}

						if(!found) 
						{

							btBones[j].second->setManuallyControlled(true);
							std::pair<Ogre::Bone*,Ogre::Vector3> tmp;
							tmp.first = btBones[j].second;
							btBones[j].second->translate(-0.1 * Ogre::Vector3(AB->getDeltaAngularVelocity().getX(),AB->getDeltaAngularVelocity().getY(),AB->getDeltaAngularVelocity().getZ()));
							btBones[j].second->setManuallyControlled(false);
							tmp.second = btBones[j].second->getPosition();
							bonesToReset.push_back(tmp);
							return btBones[j].second->getName();
						}
						
					}
				}
			}
		}
	}
	
	return "";
}

//-------------------------------------------------------------------------------------
void SkeletonToRagdoll::resetAfterOffset()
{
	for(int i = bonesToReset.size() - 1; i >= 0; i--)
	{
		Ogre::Vector3 currPos = bonesToReset[i].first->getPosition();
		bonesToReset[i].first->setManuallyControlled(true);
		bonesToReset[i].first->setPosition(bonesToReset[i].second);
		bonesToReset[i].first->translate((currPos - bonesToReset[i].second) / 10);
		bonesToReset[i].second = bonesToReset[i].first->getPosition();
		
		if(bonesToReset[i].first->getPosition().distance(currPos) < 1) 
		{
			bonesToReset[i].first->setManuallyControlled(false);
			bonesToReset.erase(bonesToReset.begin() + i);
		}			
	}
	
}

//-------------------------------------------------------------------------------------
void SkeletonToRagdoll::setBone(Ogre::Bone* _bone, btRigidBody* parentBone) 
{
	btScalar size;
	Ogre::Real sizeFactor = 2.0f;
	Ogre::Real capsuleRadius = 3.0f;
		
	if(_bone!=skeleton->getRootBone()) size = btScalar((_bone->_getDerivedPosition() * scale).distance(_bone->getParent()->_getDerivedPosition()*scale));
	else size = 1;

	if(_bone->getName() == "HEAD") 
	{
		capsuleRadius = 18.0f;
		size *= 0.9f;
	}
	else if(_bone->getName() == "HAND_LEFT" || _bone->getName() == "HAND_RIGHT") 
	{
		capsuleRadius = 5.0f;
	}
	else if(_bone->getName() == "FOOT_LEFT" || _bone->getName() == "FOOT_RIGHT") 
	{
		capsuleRadius = 5.0f;
	}
	else if(_bone->getName() == "HAIR_3") 
	{
		capsuleRadius = 4.0f;
	}

	Ogre::Entity* boneEnt = mSceneMgr->createEntity(_bone->getName(), "sphere.mesh");
	Ogre::SceneNode* boneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(_bone->getName());
	boneNode->attachObject(boneEnt);
	boneNode->setScale(capsuleRadius, capsuleRadius, size / sizeFactor);
	boneNode->setPosition(_bone->_getDerivedPosition()*scale);

	btCollisionShape* shape = new btCapsuleShapeZ(btScalar(capsuleRadius), size / sizeFactor);
	btTransform transform;
	transform.setIdentity();
		
	transform.setOrigin(btVector3(_bone->_getDerivedPosition().x * scale.x + skeletonNode->_getDerivedPosition().x,
								  _bone->_getDerivedPosition().y * scale.y + skeletonNode->_getDerivedPosition().y,
		                          _bone->_getDerivedPosition().z * scale.z + skeletonNode->_getDerivedPosition().z));

	transform.setRotation(btQuaternion(_bone->_getDerivedOrientation().getYaw().valueRadians(),
		_bone->_getDerivedOrientation().getPitch().valueRadians(),
		_bone->_getDerivedOrientation().getRoll().valueRadians()));
	
	btRigidBody* bone = localCreateRigidBody(btScalar(0), transform, shape);
	bone->setDamping(10.95, 10.85);
	bone->setDeactivationTime(0.8);
	bone->setActivationState(ACTIVE_TAG);

	std::pair<btRigidBody*,Ogre::Bone*> tmp;
	tmp.first = bone;
	tmp.second = _bone;

	btBones.push_back(tmp);
	debugBones.push_back(boneNode);
	
	for(unsigned int i = 0; i < _bone->numChildren(); i++) 
	{
		Ogre::Bone* childBone = dynamic_cast<Ogre::Bone*>(_bone->getChild(i));
		if(childBone)
			setBone(childBone,bone);
	}
}