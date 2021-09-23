#ifndef VEL_MEMORY_ALLOCATOR_CONFIG_H
#define VEL_MEMORY_ALLOCATOR_CONFIG_H

#include "MemoryManager/VELAlignedAllocator.h"

enum MemoryCategory
{
    /// General purpose
    MEMCATEGORY_GENERAL = 0,
    /// Geometry held in main memory
    MEMCATEGORY_GEOMETRY = 1,
    /// Animation data like tracks, bone matrices
    MEMCATEGORY_ANIMATION = 2,
    /// Nodes, control data
    MEMCATEGORY_SCENE_CONTROL = 3,
    /// Scene object instances
    MEMCATEGORY_SCENE_OBJECTS = 4,
    /// Other resources
    MEMCATEGORY_RESOURCE = 5,
    /// Scripting
    MEMCATEGORY_SCRIPTING = 6,
    /// Rendersystem structures
    MEMCATEGORY_RENDERSYS = 7,


    // sentinel value, do not use 
    MEMCATEGORY_COUNT = 8
};

namespace VEL
{
    class AllocPolicy {};

    // this is a template, mainly so swig does not pick it up
    template<int Category = MEMCATEGORY_GENERAL> class AllocatedObject {};

    typedef AllocatedObject<> SceneCtlAllocatedObject;
    typedef SceneCtlAllocatedObject     NodeAlloc;
    typedef SceneCtlAllocatedObject     SceneMgtAlloc;

    typedef AllocatedObject<> GeneralAllocatedObject;
    typedef AllocatedObject<> ResourceAllocatedObject;
    typedef AllocatedObject<> SceneObjAllocatedObject;
    typedef AllocatedObject<> RenderSysAllocatedObject;


    typedef GeneralAllocatedObject      RootAlloc;
    typedef ResourceAllocatedObject     ResourceAlloc;

    typedef SceneObjAllocatedObject     MovableAlloc;
    typedef ResourceAllocatedObject     ResourceAlloc;
    typedef RenderSysAllocatedObject    BufferAlloc;
#   define VEL_NEW_T(T, category) new T
}

#endif // !VEL_MEMORY_ALLOCATOR_CONFIG_H
