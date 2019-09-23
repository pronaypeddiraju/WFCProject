#pragma once
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

#include "WFCArray2D.hpp"
#include "WFCColor.hpp"
#include <optional>


//Read an image. Returns nullopt if there was an error.
std::optional<Array2D<Color>> ReadImage(const std::string& file_path) noexcept 
{
	int width;
	int height;
	int num_components;
	
	unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 3);
	
	if (data == nullptr) 
	{
		return std::nullopt;
	}
	
	Array2D<Color> imageArray = Array2D<Color>(height, width);
	
	for (unsigned i = 0; i < (unsigned)height; i++)
	{
		for (unsigned j = 0; j < (unsigned)width; j++) 
		{
			unsigned index = 3 * (i * width + j);
			imageArray.data[i * width + j] = { data[index], data[index + 1], data[index + 2] };
		}
	}

	free(data);
	return imageArray;
}

//Write image in png format 
void WriteImageAsPNG(const std::string& file_path, const Array2D<Color>& imageData) noexcept 
{
	stbi_write_png(file_path.c_str(), imageData.width, imageData.height, 3, (const unsigned char*)imageData.data.data(), 0);
}

