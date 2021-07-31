#pragma once
#ifndef BINDABLE_H
#define BINDABLE_H

#include <memory>



namespace Bind
{
	class Bindable
	{
	public:
		Bindable* Bind()
		{
			return this;
		}
		virtual ~Bindable() = default;
	};
}

#endif // !BINDABLE_H
