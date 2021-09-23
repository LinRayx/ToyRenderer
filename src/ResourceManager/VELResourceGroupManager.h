#ifndef VEL_RESOURCE_GROUP_MANAGER_H
#define VEL_RESOURCE_GROUP_MANAGER_H
#include "Template/VELSingleton.h"
#include "Utils/VELCommand.h"
#include "MemoryManager/VELMemoryAllocatorConfig.h"

#if OGRE_RESOURCEMANAGER_STRICT == 0
#   define OGRE_RESOURCE_GROUP_INIT = RGN_AUTODETECT
#elif OGRE_RESOURCEMANAGER_STRICT == 1
#   define OGRE_RESOURCE_GROUP_INIT
#else
#   define OGRE_RESOURCE_GROUP_INIT = RGN_DEFAULT
#endif


namespace VEL
{
    /// Default resource group name
    extern const char* const RGN_DEFAULT;
    /// Internal resource group name (should be used by OGRE internal only)
    extern const char* const RGN_INTERNAL;
    /// Special resource group name which causes resource group to be automatically determined based on searching for the resource in all groups.
    extern const char* const RGN_AUTODETECT;

	class ResourceGroupManager : public Singleton<ResourceGroupManager>, public ResourceAlloc
	{
    public:
        /// same as @ref RGN_DEFAULT
        static const String DEFAULT_RESOURCE_GROUP_NAME;
        /// same as @ref RGN_INTERNAL
        static const String INTERNAL_RESOURCE_GROUP_NAME;
        /// same as @ref RGN_AUTODETECT
        static const String AUTODETECT_RESOURCE_GROUP_NAME;
        /// The number of reference counts held per resource by the resource system
        static const long RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS;
	};
}

#endif // !VEL_RESOURCE_GROUP_MANAGER_H

