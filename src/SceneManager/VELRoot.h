#ifndef VEL_ROOT_H
#define VEL_ROOT_H

#include "Template/VELSingleton.h"
#include "MemoryManager/VELMemoryAllocatorConfig.h"
#include "SceneManager/VELMovableObject.h"

namespace VEL
{
	class Root : public Singleton<Root>, public RootAlloc
	{
	public:
		typedef std::map<String, MovableObjectFactory*> MovableObjectFactoryMap;
	public:
		Root(const String& pluginFileName = "plugins.cfg",
			const String& configFileName = "ogre.cfg",
			const String& logFileName = "Ogre.log");
		~Root();

		MovableObjectFactory* getMovableObjectFactory(const String& typeName);
	private:
		MovableObjectFactoryMap mMovableObjectFactoryMap;
	};
}

#endif // !VEL_ROOT_H
