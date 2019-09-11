#pragma once
#include <functional>

//Represent color in 24 bit RGB
struct Color 
{
	unsigned char r, g, b;

	bool operator==(const Color &color) const noexcept 
	{
		return r == color.r && g == color.g && b == color.b;
	}

	bool operator!=(const Color &color) const noexcept
	{ 
		return !(color == *this);
	}
};

//Hash function for color
template <> class std::hash<Color>
{
public:
	size_t operator()(const Color &color) const
	{
		return (size_t)color.r +
			(size_t)256 * (size_t)color.g +
			(size_t)256 * (size_t)256 * (size_t)color.b;
	}
};

