#include "ResourceManager/VELResourceManager.h"

namespace VEL
{
	ResourceManager::ResourceCreateOrRetrieveResult ResourceManager::createOrRetrieve(const String& name, const String& group, bool isManual, ManualResourceLoader* loader, const NameValuePairList* params)
	{
        ResourcePtr res = getResourceByName(name, group);
        bool created = false;
        if (!res)
        {
            created = true;
            res = createResource(name, group, isManual, loader, params);
        }

        return ResourceCreateOrRetrieveResult(res, created);
	}
    ResourcePtr ResourceManager::getResourceByName(const String& name, const String& groupName OGRE_RESOURCE_GROUP_INIT) const
    {
        return ResourcePtr();
    }
    ResourcePtr ResourceManager::createResource(const String& name, const String& group, bool isManual, ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        return ResourcePtr();
    }
}
