#include "Utils/VELStdHeaders.h"

namespace VEL
{
	typedef std::string String;
	class Mesh;
	class Node;
	class SceneManager;
	class Resource;
	class DataStream;

	typedef std::shared_ptr<Mesh> MeshPtr;
	typedef std::shared_ptr<Resource> ResourcePtr;
	typedef std::shared_ptr<DataStream> DataStreamPtr;

	typedef unsigned long ulong;

	using std::static_pointer_cast;
}