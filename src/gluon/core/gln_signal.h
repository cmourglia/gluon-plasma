#pragma once

#include <EASTL/vector.h>
#include <EASTL/functional.h>

namespace gluon
{

struct signal
{
	using callback  = eastl::function<void(void)>;
	using callbacks = eastl::vector<callback>;

	callbacks Callbacks;

	void Subscribe(callback&& Callback) { Callbacks.push_back(eastl::move(Callback)); }

	void Fire()
	{
		for (auto&& Callback : Callbacks)
		{
			Callback();
		}
	}
};

}
