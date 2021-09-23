#ifndef VEL_SCENE_MANAGER_H
#define VEL_SCENE_MANAGER_H

#include "MemoryManager/VELMemoryAllocatorConfig.h"
#include "Utils/VELCommand.h"
#include "SceneManager/VELRoot.h"
#include "SceneManager/VELEntity.h"
#include "ResourceManager/VELResourceGroupManager.h"

namespace VEL
{
	class SceneManager : public SceneMgtAlloc
	{
	public:
		typedef std::map<String, MovableObject*> MovableObjectMap;
	protected:
		String mName;

		struct MovableObjectCollection
		{
			MovableObjectMap map;
		};
		typedef std::map<String, MovableObjectCollection*> MovableObjectCollectionMap;
		MovableObjectCollectionMap mMovableObjectCollectionMap;
	public:
		/** Constructor.
*/
		SceneManager(const String& instanceName);

		/** Default destructor.
		*/
		virtual ~SceneManager();

		Entity* createEntity(const String& entityName, const String& meshName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		MovableObject* createMovableObject(const String& name,
			const String& typeName, const NameValuePairList* params = 0);

		MovableObjectCollection* getMovableObjectCollection(const String& typeName);
	};
}

#endif // !VEL_SCENE_MANAGER_H
