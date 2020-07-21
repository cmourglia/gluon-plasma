#include "gln_observable.h"

#include <EASTL/vector.h>
#include <EASTL/string_hash_map.h>

namespace gluon 
{
	using callbacks = eastl::string_hash_map<eastl::vector<observable::callback>>;

	struct observable_impl 
	{
		callbacks Callbacks;
	};

	void observable::Notify(const char* Property)
	{
		for (auto&& c : m_Impl->Callbacks[Property])
			c(this);
	}

	void observable::Subscribe(const char* Property, callback&& Callback) 
	{
		m_Impl->Callbacks[Property].push_back(eastl::move(Callback));
	}
};