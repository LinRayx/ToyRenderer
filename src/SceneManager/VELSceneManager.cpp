#include "SceneManager/VELSceneManager.h"

namespace VEL
{
	SceneManager::SceneManager(const String& instanceName)
		: mName(instanceName)
	{
		Root* root = Root::getSingletonPtr();
	}
	SceneManager::~SceneManager()
	{
	}
	Entity* SceneManager::createEntity(const String& entityName, const String& meshName, const String& groupName)
	{
		return nullptr;
	}
	MovableObject* SceneManager::createMovableObject(const String& name, const String& typeName, const NameValuePairList* params)
	{
		if (typeName == "Camera") {

		}
		MovableObjectFactory* factory =
			Root::getSingleton().getMovableObjectFactory(typeName);
		MovableObjectCollection* objectMap = getMovableObjectCollection(typeName);

		if (objectMap->map.find(name) != objectMap->map.end())
		{
			VEL_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
				"An object of type '" + typeName + "' with name '" + name
				+ "' already exists.",
				"SceneManager::createMovableObject");
		}

		MovableObject* newObj = factory->createInstance(name, this, params);
		objectMap->map[name] = newObj;
		return newObj;

	}
	SceneManager::MovableObjectCollection* SceneManager::getMovableObjectCollection(const String& typeName)
	{
		MovableObjectCollectionMap::iterator i =
			mMovableObjectCollectionMap.find(typeName);
		if (i == mMovableObjectCollectionMap.end())
		{
			// create
			MovableObjectCollection* newCollection = VEL_NEW_T(MovableObjectCollection, MEMCATEGORY_SCENE_CONTROL)();
			mMovableObjectCollectionMap[typeName] = newCollection;
			return newCollection;
		}
		else
		{
			return i->second;
		}
	}
}