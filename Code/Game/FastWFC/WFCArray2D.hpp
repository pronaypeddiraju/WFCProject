#pragma once
#include <vector>
#include "Engine/Commons/ErrorWarningAssert.hpp"

//------------------------------------------------------------------------------------------------------------------------------
template <typename T> class Array2D
{

public:
	unsigned int m_height;
	unsigned int m_width;

	std::vector<T> m_data;

	//Build a 2D array given its height and width.
	//All the array elements are initialized to default value.
	Array2D(unsigned int height, unsigned int width) noexcept
		: m_height(height), m_width(width), m_data(width * height) {}

	//Build a 2D array given its height and width.
	//All the array elements are initialized to value.
	Array2D(unsigned int height, unsigned int width, T value) noexcept
		: m_height(height), m_width(width), m_data(width * height, value) {}

	//Return a const reference to the element in the i-th line and j-th column.
	//i must be lower than height and j lower than width.
	const T &Get(unsigned int i, unsigned int j) const noexcept
	{
		ASSERT_OR_DIE((i < m_height && j < m_width), "Trying to get from array for indices greater than array size");
		return m_data[j + i * m_width];
	}

	//Return a reference to the element in the i-th line and j-th column.
	//i must be lower than height and j lower than width.
	T &Get(unsigned int i, unsigned int j) noexcept 
	{
		ASSERT_OR_DIE((i < m_height && j < m_width), "Trying to get from array for indices greater than array size");
		return m_data[j + i * m_width];
	}

	//Return the current 2D array reflected along the x axis.
	Array2D<T> GetReflected() const noexcept 
	{
		Array2D<T> result = Array2D<T>(m_width, m_height);
		for (unsigned int y = 0; y < m_height; y++) 
		{
			for (unsigned int x = 0; x < m_width; x++) 
			{
				result.Get(y, x) = Get(y, m_width - 1 - x);
			}
		}
		return result;
	}

	
	//Return the current 2D array rotated 90° anticlockwise
	Array2D<T> GetRotated() const noexcept 
	{
		Array2D<T> result = Array2D<T>(m_width, m_height);
		for (unsigned int y = 0; y < m_width; y++) 
		{
			for (unsigned int x = 0; x < m_height; x++) 
			{
				result.Get(y, x) = Get(x, m_width - 1 - y);
			}
		}
		return result;
	}

	//Return the sub 2D array starting from (y,x) and with size (sub_width,
	//sub_height). The current 2D array is considered toric for this operation.
	Array2D<T> GetSubArray(unsigned int y, unsigned int x, unsigned int sub_width,
		unsigned int sub_height) const noexcept 
	{
		Array2D<T> sub_array_2d = Array2D<T>(sub_width, sub_height);
		for (unsigned int ki = 0; ki < sub_height; ki++) 
		{
			for (unsigned int kj = 0; kj < sub_width; kj++) 
			{
				sub_array_2d.Get(ki, kj) = Get((y + ki) % m_height, (x + kj) % m_width);
			}
		}
		return sub_array_2d;
	}

	//Assign the matrix a to the current matrix.
	Array2D<T> &operator=(const Array2D<T> &a) noexcept 
	{
		m_height = a.m_height;
		m_width = a.m_width;
		m_data = a.m_data;
		return *this;
	}

	//Check if two 2D arrays are equals.
	bool operator==(const Array2D<T> &a) const noexcept 
	{
		if (m_height != a.m_height || m_width != a.m_width) 
		{
			return false;
		}

		for (unsigned int i = 0; i < m_data.size(); i++) 
		{
			if (a.m_data[i] != m_data[i]) 
			{
				return false;
			}
		}
		return true;
	}
};

//------------------------------------------------------------------------------------------------------------------------------
//Extending the hash function for case of type T = Array2D
//Math magic number: 0x9e3779b9 as per the internet
//This only exists so I can store an unordered map of Array2Ds
//because std library needs a function to generate a hash to store
//it in the unordered map
//------------------------------------------------------------------------------------------------------------------------------
namespace std
{
	template <typename T> class hash<Array2D<T>>
	{
	public:
		size_t operator()(const Array2D<T> &a) const noexcept
		{
			std::size_t seed = a.m_data.size();
			for (const T &i : a.m_data)
			{
				seed = seed ^ (hash<T>()(i) + (size_t)0x9e3779b9 + (seed << 6) + (seed >> 2));
			}
			return seed;
		}
	};
} // namespace std