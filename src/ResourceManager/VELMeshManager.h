#ifndef VEL_MESH_MANAGER_H
#define VEL_MESH_MANAGER_H
#include "ResourceManager/VELResourceManager.h"
#include "Template/VELSingleton.h"
#include "ResourceManager/VELHardwareBuffer.h"
#include "Utils/VELCommand.h"
#include "ResourceManager/VELMesh.h"

namespace VEL
{
	class MeshManager : public ResourceManager, public Singleton<MeshManager>
	{
	public:
		MeshManager();
		~MeshManager();

		MeshPtr load(const String& filename, const String& groupName,
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool vertexBufferShadowed = false, bool indexBufferShadowed = false);
		using ResourceManager::createOrRetrieve;

		ResourceCreateOrRetrieveResult createOrRetrieve(
			const String& name,
			const String& group,
			bool isManual, ManualResourceLoader* loader,
			const NameValuePairList* createParams,
			HardwareBuffer::Usage vertexBufferUsage,
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool vertexBufferShadowed = false, bool indexBufferShadowed = false);
	};
}

#endif // !VEL_MESH_MANAGER_H
