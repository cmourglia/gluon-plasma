#pragma once

#include <EASTL/functional.h>

namespace gluon 
{

struct observable_impl;

struct observable
{
	using callback = eastl::function<void (observable*)>;

	void Notify(const char* Property);
	void Subscribe(const char* Property, callback&& Callback);

private:
	observable_impl* m_Impl = nullptr;
};

};