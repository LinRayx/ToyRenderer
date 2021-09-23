#include "SceneManager/VELEntity.h"

namespace VEL
{
	MovableObject* EntityFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
	{
        // must have mesh parameter
        MeshPtr pMesh;
        if (params != 0)
        {
            String groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;

            NameValuePairList::const_iterator ni;

            ni = params->find("resourceGroup");
            if (ni != params->end())
            {
                groupName = ni->second;
            }

            ni = params->find("mesh");
            if (ni != params->end())
            {
                // Get mesh (load if required)
                pMesh = MeshManager::getSingleton().load(
                    ni->second,
                    // autodetect group location
                    groupName);
            }

        }
        if (!pMesh)
        {
            VEL_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "'mesh' parameter required when constructing an Entity.",
                "EntityFactory::createInstance");
        }

        return VEL_NEW Entity(name, pMesh);
	}
	const String& EntityFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
}