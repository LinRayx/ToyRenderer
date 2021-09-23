#include "SceneManager/VELRoot.h"

namespace VEL
{
	template<> Root* Singleton<Root>::msSingleton = 0;
	Root::Root(const String& pluginFileName, const String& configFileName, const String& logFileName)
	{
	}

	Root::~Root()
	{
	}
	MovableObjectFactory* Root::getMovableObjectFactory(const String& typeName)
	{
		MovableObjectFactoryMap::iterator i =
			mMovableObjectFactoryMap.find(typeName);
		if (i == mMovableObjectFactoryMap.end())
		{
			VEL_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"MovableObjectFactory of type " + typeName + " does not exist",
				"Root::getMovableObjectFactory");
		}
		return i->second;
	}
}