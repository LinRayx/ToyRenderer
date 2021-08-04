#ifndef DESCRIPTOR_SETS_H
#define DESCRIPTOR_SETS_H
#include "Bindable.h"
#include "Buffer.h"
#include <vector>
#include <memory>
using namespace std;

namespace Bind
{
	class DescriptorSets : public Bindable
	{
		struct DescInfo
		{
			shared_ptr<Graphics::Buffer> buffer_ptr;
			Graphics::BufferUsage usage;
		};
	public:
		void Add(shared_ptr<Graphics::Buffer> buffer_ptr, Graphics::BufferUsage usage)
		{
			descInfos.push_back();
		}
	private:
		vector<DescInfo> descInfos;
	};
}

#endif // !DESCRIPTOR_SETS_H