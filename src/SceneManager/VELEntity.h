#ifndef VEL_ENTITY_H
#define VEL_ENTITY_H
#include "Utils/VELCommand.h"
#include "SceneManager/VELMovableObject.h"
#include "ResourceManager/VELResourceGroupManager.h"
#include "MemoryManager/VELMemoryAllocatorConfig.h"

namespace VEL
{
	class Entity : public MovableObject
	{
		friend class EntityFactory;
	public:
		typedef std::set<Entity*> EntitySet;
	private:
		/** Private constructor (instances cannot be created directly).
		*/
		Entity();
		/** Private constructor - specify name (the usual constructor used).
		*/
		Entity(const String& name, const MeshPtr& mesh);

		/** The Mesh that this Entity is based on.
		*/
		MeshPtr mMesh;
	protected:
		/// Name of this object
		String mName;
	};

	class EntityFactory
	{
	private:
		MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
	public:
		EntityFactory() {}
		~EntityFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
	};
}

#endif // !VEL_ENTITY_H
