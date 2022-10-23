#pragma once

template<typename T>
class ChangeDetector
{
	T currentValue;
	T previousValue;
	public:
	ChangeDetector()
		: currentValue(T()), previousValue(T())
	{}
	ChangeDetector(const T& value)
		: currentValue(value), previousValue(value)
	{}
	const T& GetValue() const noexcept
	{
		return currentValue;
	}
	bool IsChanged()  const noexcept
	{
		return currentValue != previousValue;
	}
	void SetValue(const T& value) noexcept
	{
		previousValue = currentValue;
		currentValue = value;
	}
	operator T() const noexcept
	{
		return currentValue;
	}

	ChangeDetector& operator=(const T& rhs) noexcept
	{
		SetValue(rhs);
		return *this;
	}

};
