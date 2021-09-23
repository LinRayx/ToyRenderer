#include "ResourceManager/VELResourceGroupManager.h"

namespace VEL
{
    const char* const RGN_DEFAULT = "General";
    const char* const RGN_INTERNAL = "OgreInternal";
    const char* const RGN_AUTODETECT = "OgreAutodetect";

    const String ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME = RGN_DEFAULT;
    const String ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME = RGN_INTERNAL;
    const String ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME = RGN_AUTODETECT;

    // A reference count of 3 means that only RGM and RM have references
    // RGM has one (this one) and RM has 2 (by name and by handle)
    const long ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS = 3;
}