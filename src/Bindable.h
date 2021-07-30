#pragma once
#ifndef BINDABLE_H
#define BINDABLE_H

#include "Pipeline.h"
#include "Vulkan.h"
#include <memory>

namespace Bind
{
	class Bindable
	{
	public:
		virtual void Bind(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr) noexcept = 0;
		virtual ~Bindable() = default;
	protected:
	};
}

#endif // !BINDABLE_H
