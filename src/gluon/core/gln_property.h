#pragma once

#include <EASTL/vector.h>
#include <EASTL/functional.h>

namespace gluon
{

template <typename T>
struct property
{
	using callback  = eastl::function<void(const T&)>;
	using callbacks = eastl::vector<callback>;

	T         Data;
	callbacks Callbacks;

	property() = default;
	property(const T& Data) { this->Data = Data; }
	property(const property<T>& Other)
	{
		Data      = Other.Data;
		Callbacks = Other.Callbacks;
	}
	// property(property<T>&& Other)
	// {
	// 	Data      = eastl::move(Other.Data);
	// 	Callbacks = eastl::move(Other.Callbacks);
	// }

	~property() = default;

	property<T>& operator=(const T& Data)
	{
		this->Data = Data;
		Notify();

		return *this;
	}

	// property<T>& operator=(property<T>&& Other)
	// {
	// 	this->Data      = eastl::move(Other.Data);
	// 	this->Callbacks = eastl::move(Callbacks);
	// 	Notify();

	// 	return *this;
	// }

	property<T>& operator=(property<T>& Other)
	{
		this->Data = Other.Data;
		Other.Subscribe([this](const T& Data) {
			this->Data = Data;
			this->Notify();
		});

		Notify();

		return *this;
	}

	void Subscribe(callback&& Callback) { Callbacks.push_back(eastl::move(Callback)); }

	void Notify()
	{
		for (auto&& Callback : Callbacks)
		{
			Callback(Data);
		}
	}
};

}