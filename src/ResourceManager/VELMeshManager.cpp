#include "ResourceManager/VELMeshManager.h"

namespace VEL
{
	MeshManager::MeshManager()
	{
	}
	MeshManager::~MeshManager()
	{
	}
	MeshPtr MeshManager::load(const String& filename, const String& groupName, HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage, bool vertexBufferShadowed, bool indexBufferShadowed)
	{
		MeshPtr pMesh = static_pointer_cast<Mesh>(createOrRetrieve(filename, groupName, false, 0, 0,
			vertexBufferUsage, indexBufferUsage,
			vertexBufferShadowed, indexBufferShadowed).first);
		pMesh->load();
		return pMesh;
	}
	MeshManager::ResourceCreateOrRetrieveResult MeshManager::createOrRetrieve(const String& name, const String& group, bool isManual, ManualResourceLoader* loader, const NameValuePairList* createParams, HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage, bool vertexBufferShadowed, bool indexBufferShadowed)
	{
		ResourceCreateOrRetrieveResult res =
			ResourceManager::createOrRetrieve(name, group, isManual, loader, createParams);
		MeshPtr pMesh = static_pointer_cast<Mesh>(res.first);
		return ResourceCreateOrRetrieveResult();
	}
}