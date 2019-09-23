#pragma once
#include <vector>
#include "Engine/Commons/ErrorWarningAssert.hpp"


template <typename T> class Array2D 
{

	public:
		/**
		 * Height and width of the 2D array.
		 */
		unsigned int height;
		unsigned int width;

		/**
		 * The array containing the data of the 2D array.
		 */
		std::vector<T> data;

		/**
		 * Build a 2D array given its height and width.
		 * All the array elements are initialized to default value.
		 */
		Array2D(unsigned int height, unsigned int width) noexcept
			: height(height), width(width), data(width * height) {}

		/**
		 * Build a 2D array given its height and width.
		 * All the array elements are initialized to value.
		 */
		Array2D(unsigned int height, unsigned int width, T value) noexcept
			: height(height), width(width), data(width * height, value) {}

		/**
		 * Return a const reference to the element in the i-th line and j-th column.
		 * i must be lower than height and j lower than width.
		 */
		const T &get(unsigned int i, unsigned int j) const noexcept {
			ASSERT_OR_DIE((i < height && j < width), "Trying to get from array for indices greater than array size");
			return data[j + i * width];
		}

		/**
		 * Return a reference to the element in the i-th line and j-th column.
		 * i must be lower than height and j lower than width.
		 */
		T &get(unsigned int i, unsigned int j) noexcept {
			ASSERT_OR_DIE((i < height && j < width), "Trying to get from array for indices greater than array size");
			return data[j + i * width];
		}

		/**
		 * Return the current 2D array reflected along the x axis.
		 */
		Array2D<T> reflected() const noexcept {
			Array2D<T> result = Array2D<T>(width, height);
			for (unsigned int y = 0; y < height; y++) {
				for (unsigned int x = 0; x < width; x++) {
					result.get(y, x) = get(y, width - 1 - x);
				}
			}
			return result;
		}

		/**
		 * Return the current 2D array rotated 90� anticlockwise
		 */
		Array2D<T> rotated() const noexcept {
			Array2D<T> result = Array2D<T>(width, height);
			for (unsigned int y = 0; y < width; y++) {
				for (unsigned int x = 0; x < height; x++) {
					result.get(y, x) = get(x, width - 1 - y);
				}
			}
			return result;
		}

		/**
		 * Return the sub 2D array starting from (y,x) and with size (sub_width,
		 * sub_height). The current 2D array is considered toric for this operation.
		 */
		Array2D<T> GetSubArray(unsigned int y, unsigned int x, unsigned int sub_width,
			unsigned int sub_height) const noexcept {
			Array2D<T> sub_array_2d = Array2D<T>(sub_width, sub_height);
			for (unsigned int ki = 0; ki < sub_height; ki++) {
				for (unsigned int kj = 0; kj < sub_width; kj++) {
					sub_array_2d.get(ki, kj) = get((y + ki) % height, (x + kj) % width);
				}
			}
			return sub_array_2d;
		}

		/**
		 * Assign the matrix a to the current matrix.
		 */
		Array2D<T> &operator=(const Array2D<T> &a) noexcept {
			height = a.height;
			width = a.width;
			data = a.data;
			return *this;
		}

		/**
		 * Check if two 2D arrays are equals.
		 */
		bool operator==(const Array2D<T> &a) const noexcept {
			if (height != a.height || width != a.width) {
				return false;
			}

			for (unsigned int i = 0; i < data.size(); i++) {
				if (a.data[i] != data[i]) {
					return false;
				}
			}
			return true;
		}
};

//Extending the hash function for case of type T = Array2D
//Math magic number: MATH_TEA = 0x9e3779b9 as per the internet
//This only exists so I can store an unordered map of Array2Ds
//because std library needs a function to generate a hash to store
//it in the unordered map
namespace std 
{
	template <typename T> class hash<Array2D<T>> 
	{
		public:
			size_t operator()(const Array2D<T> &a) const noexcept 
			{
				std::size_t seed = a.data.size();
				for (const T &i : a.data) 
				{
					seed = seed ^ (hash<T>()(i) + (size_t)0x9e3779b9 + (seed << 6) + (seed >> 2));
				}
				return seed;
			}
	};
} // namespace std