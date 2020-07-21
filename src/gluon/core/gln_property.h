#pragma once

#define USE_STD 0

#if USE_STD
#include <vector.h>
#include <functional.h>
#define lib std
#else
#include <EASTL/vector.h>
#include <EASTL/functional.h>
#define lib eastl
#endif

template<typename T>
struct property 
{
	using callback = lib::function<void (const T&)>;
	using callbacks = lib::vector<callback>;
	
	T Data;
	callbacks Callbacks;

	void operator=(const T& Data) 
	{
		Data = Data;
		Notify();
	}

	void Subscribe(const callback& Callback)
	{
		Callbacks.push_back(Callback);
	}

	void Notify() 
	{
		for (auto&& Callback : Callbacks)
		{
			Callback(Data);
		}
	}
};