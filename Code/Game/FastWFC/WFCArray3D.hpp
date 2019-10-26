#pragma once
#include "assert.h"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------
template <typename T> class Array3D 
{

public:
	unsigned int m_height;
	unsigned int m_width;
	unsigned int m_depth;

	std::vector<T> m_data;

	//Build a 2D array given its height, width and depth.
	//All the arrays elements are initialized to default value.
	Array3D(unsigned int height, unsigned int width, unsigned int depth) noexcept
		: m_height(height), m_width(width), m_depth(depth),
		m_data(width * height * depth) {}

	//Build a 2D array given its height, width and depth.
	//All the arrays elements are initialized to value
	Array3D(unsigned int height, unsigned int width, unsigned int depth, T value) noexcept
		: m_height(height), m_width(width), m_depth(depth),
		m_data(width * height * depth, value) {}

	/**
	 * Return a const reference to the element in the i-th line, j-th column, and
	 * k-th depth. i must be lower than height, j lower than width, and k lower
	 * than depth.
	 */
	const T &Get(unsigned int i, unsigned int j, unsigned int k) const noexcept 
	{
		assert(i < m_height && j < m_width && k < m_depth);
		return m_data[i * m_width * m_depth + j * m_depth + k];
	}

	
	//Return a reference to the element in the i-th line, j-th column, and k-th
	//depth. i must be lower than height, j lower than width, and k lower than
	//depth.
	T &Get(unsigned int i, unsigned int j, unsigned int k) noexcept 
	{
		return m_data[i * m_width * m_depth + j * m_depth + k];
	}

	//Check if two 3D arrays are equals.
	bool operator==(const Array3D &a) const noexcept 
	{
		if (m_height != a.m_height || m_width != a.m_width || m_depth != a.m_depth) 
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
