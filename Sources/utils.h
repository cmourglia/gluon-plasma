#pragma once

template <typename T>
class Optional
{
public:
	Optional()
	    : m_HasValue(false)
	{
	}

	Optional(const T& Value)
	    : m_Value(Value)
	    , m_HasValue(true)
	{
	}

	Optional(const Optional<T>& Other)
	    : m_Value(Other.m_Value)
	    , m_HasValue(Other.m_HasValue)
	{
	}

	Optional(Optional<T>&& Other)
	    : m_Value(std::move(Other.m_Value))
	    , m_HasValue(Other.m_HasValue)
	{
	}

	Optional& operator=(const T& Value)
	{
		m_Value    = Value;
		m_HasValue = true;

		return *this;
	}

	Optional& operator=(const Optional<T>& Other)
	{
		m_Value    = Other.m_Value;
		m_HasValue = Other.m_HasValue;

		return *this;
	}

	Optional& operator=(Optional<T>&& Other)
	{
		m_Value    = std::move(Other.m_Value);
		m_HasValue = Other.m_HasValue;

		return *this;
	}

	bool HasValue() const
	{
		return m_HasValue;
	}

	const T& Value() const
	{
		return m_Value;
	}

	T& Value()
	{
		return m_Value;
	}

	bool operator==(const Optional<T>& Other) const
	{
		if (Other.m_HasValue() && m_HasValue)
		{
			return m_Value == Other.m_Value;
		}

		return false;
	}

	bool operator!=(const Optional<T>& Other) const
	{
		if (!m_HasValue || !Other.m_HasValue)
		{
			return true;
		}

		return m_Value != Other.m_Value;
	}

private:
	T    m_Value;
	bool m_HasValue;
};

template <typename T>
constexpr inline T Min(T A, T B)
{
	return A < B ? A : B;
}

template <typename T>
constexpr inline T Max(T A, T B)
{
	return A > B ? A : B;
}

template <typename T>
constexpr inline T Clamp(T X, T A, T B)
{
	return Max(A, Min(B, X));
}
