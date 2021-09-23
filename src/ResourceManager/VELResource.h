#ifndef VEL_RESOURCE_H
#define VEL_RESOURCE_H

namespace VEL
{
	class Resource
	{
	public:
		virtual void load(bool backgroundThread = false);
	};

	class ManualResourceLoader
	{

	};
}

#endif // !VEL_RESOURCE_H
