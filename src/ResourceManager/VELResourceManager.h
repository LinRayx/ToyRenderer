#ifndef VEL_RESOURCE_MANAGER_H
#define VEL_RESOURCE_MANAGER_H
#include "MemoryManager/VELMemoryAllocatorConfig.h"
#include "ResourceManager/VELResource.h"
#include "ResourceManager/VELResourceGroupManager.h"
#include "Utils/VELCommand.h"

namespace VEL
{
	class ResourceManager : public ResourceAlloc
	{
	public:
		typedef std::pair<ResourcePtr, bool> ResourceCreateOrRetrieveResult;

		ResourceCreateOrRetrieveResult createOrRetrieve(const String& name,
			const String& group, bool isManual = false,
			ManualResourceLoader* loader = 0,
			const NameValuePairList* createParams = 0);

		virtual ResourcePtr getResourceByName(const String& name, const String& groupName OGRE_RESOURCE_GROUP_INIT) const;
		ResourcePtr createResource(const String& name, const String& group,
			bool isManual = false, ManualResourceLoader* loader = 0,
			const NameValuePairList* createParams = 0);

	};
}

#endif // !VEL_RESOURCE_MANAGER_H
