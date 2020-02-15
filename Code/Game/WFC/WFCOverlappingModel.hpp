#pragma once
#include <vector>
#include <unordered_map>

#include "Game/WFC/WFCArray2D.hpp"
#include "Game/WFC/WFC.hpp"
#include "Game/WFC/WFCColor.hpp"

//------------------------------------------------------------------------------------------------------------------------------
//Options needed for Overlapping WFC problem
struct OverlappingWFCOptions
{
	bool m_periodicInput;  // True if the input is toric(wrapping on x and y).
	bool m_periodicOutput; // True if the output is toric(wrapping on x and y).
	unsigned m_outHeight;  // The height of the output in pixels.
	unsigned m_outWidth;   // The width of the output in pixels.
	unsigned m_symmetry; // The number of symmetries (the order is defined in wfc).
	bool m_ground;       // True if the ground needs to be set (see InitializeGround).
	unsigned m_patternSize; // The width and height in pixel of the patterns.

	//get the wave height given these options
	unsigned GetWaveHeight() const noexcept
	{
		return m_periodicOutput ? m_outHeight : m_outHeight - m_patternSize + 1;
	}

	//Get the wave width given these options
	unsigned GetWaveWidth() const noexcept
	{
		return m_periodicOutput ? m_outWidth : m_outWidth - m_patternSize + 1;
	}
};

//------------------------------------------------------------------------------------------------------------------------------
//Class generating a new image with the overlapping WFC algorithm
//------------------------------------------------------------------------------------------------------------------------------
class OverlappingWFC
{

private:
	//The input image. T is usually a RGBA or WFCColor
	Array2D<Color> m_input;

	//Options needed by algorithm
	OverlappingWFCOptions m_options;

	//Array of different patterns extracted from the input
	std::vector<Array2D<Color>> m_patterns;

	//Underlying generic WFC algorithm
	WFC m_wfc;

	//Initialize WFC. NOTE: initialize this only once
	OverlappingWFC(
		const Array2D<Color> &input, const OverlappingWFCOptions &options,
		const int &seed,
		const std::pair<std::vector<Array2D<Color>>, std::vector<double>> &patterns,
		const std::vector<std::array<std::vector<unsigned>, 4>>
		&propagator) noexcept
		: m_input(input), m_options(options), m_patterns(patterns.first),
		m_wfc(options.m_periodicOutput, seed, patterns.second, propagator,
			options.GetWaveHeight(), options.GetWaveWidth())
	{
		// If necessary, the ground is set.
		if (options.m_ground)
		{
			InitializeGround(m_wfc, input, patterns.first, options);
		}
	}

	//Calls other constructor with more computed params
	OverlappingWFC(const Array2D<Color> &input, const OverlappingWFCOptions &options,
		const int &seed,
		const std::pair<std::vector<Array2D<Color>>, std::vector<double>> &patterns)
		: OverlappingWFC(input, options, seed, patterns,
			GenerateCompatible(patterns.first))
	{

	}

	//Init the ground of the output image.
	//The lowest middle pattern is used as a floor (and ceiling when the input is
	//toric) and is placed at the lowest possible pattern position in the output
	//image, on all its width. The pattern cannot be used at any other place in
	//the output image.
	static void InitializeGround(WFC &wfc, const Array2D<Color> &input,
		const std::vector<Array2D<Color>> &patterns,
		const OverlappingWFCOptions &options) noexcept
	{
		unsigned ground_pattern_id = GetGroundPatternID(input, patterns, options);

		// Place the pattern in the ground.
		for (unsigned j = 0; j < options.GetWaveWidth(); j++)
		{
			for (unsigned p = 0; p < patterns.size(); p++)
			{
				if (ground_pattern_id != p)
				{
					wfc.RemoveWavePattern(options.GetWaveHeight() - 1, j, p);
				}
			}
		}

		// Remove the pattern from the other positions.
		for (unsigned i = 0; i < options.GetWaveHeight() - 1; i++)
		{
			for (unsigned j = 0; j < options.GetWaveWidth(); j++)
			{
				wfc.RemoveWavePattern(i, j, ground_pattern_id);
			}
		}

		// Propagate the information with wfc.
		wfc.Propagate();
	}

	//Return the id of the lowest middle pattern
	static unsigned
		GetGroundPatternID(const Array2D<Color> &input,
			const std::vector<Array2D<Color>> &patterns,
			const OverlappingWFCOptions &options) noexcept
	{
		// Get the pattern.
		Array2D<Color> ground_pattern = input.GetSubArray(input.m_height - 1, input.m_width / 2, options.m_patternSize, options.m_patternSize);

		// Retrieve the id of the pattern.
		for (unsigned i = 0; i < patterns.size(); i++)
		{
			if (ground_pattern == patterns[i])
			{
				return i;
			}
		}

		// The pattern exists.
		assert(false);
		return 0;
	}

	//Return list of patterns as well as their probabilities of appearing
	static std::pair<std::vector<Array2D<Color>>, std::vector<double>> GetPatterns(const Array2D<Color> &input, const OverlappingWFCOptions &options)
	{
		std::unordered_map<Array2D<Color>, uint> patterns_id;
		std::vector<Array2D<Color>> patterns;

		// The number of times a pattern is seen in the input image.
		std::vector<double> patterns_weight;

		std::vector<Array2D<Color>> symmetries(8, Array2D<Color>(options.m_patternSize, options.m_patternSize));

		uint max_i = options.m_periodicInput ? input.m_height : input.m_height - options.m_patternSize + 1;
		uint max_j = options.m_periodicInput ? input.m_width : input.m_width - options.m_patternSize + 1;

		for (unsigned i = 0; i < max_i; i++)
		{
			for (unsigned j = 0; j < max_j; j++)
			{
				// Compute the symmetries of every pattern in the image.
				symmetries[0].m_data = input.GetSubArray(i, j, options.m_patternSize, options.m_patternSize).m_data;
				symmetries[1].m_data = symmetries[0].GetReflected().m_data;
				symmetries[2].m_data = symmetries[0].GetRotated().m_data;
				symmetries[3].m_data = symmetries[2].GetReflected().m_data;
				symmetries[4].m_data = symmetries[2].GetRotated().m_data;
				symmetries[5].m_data = symmetries[4].GetReflected().m_data;
				symmetries[6].m_data = symmetries[4].GetRotated().m_data;
				symmetries[7].m_data = symmetries[6].GetReflected().m_data;

				// The number of symmetries in the option class define which symetries
				// will be used.
				for (uint k = 0; k < options.m_symmetry; k++)
				{
					//unordered map insert returns a pair of iterator and bool 
					std::pair<std::unordered_map<Array2D<Color>, uint>::iterator, bool> res;
					res = patterns_id.insert(std::make_pair(symmetries[k], (uint)patterns.size()));

					// If the pattern already exist, we just have to increase its number
					// of appearance.
					if (!res.second)
					{
						patterns_weight[res.first->second] += 1;
					}
					else
					{
						patterns.push_back(symmetries[k]);
						patterns_weight.push_back(1);
					}
				}
			}
		}

		return { patterns, patterns_weight };
	}

	//Return true if the pattern1 is compatible with patter2
	//pattern2 is at a distance dy,dx from pattern1
	static bool IsPatternCompatibleWithThisPattern(const Array2D<Color> &pattern1, const Array2D<Color> &pattern2, int dy, int dx)
	{
		uint xmin = dx < 0 ? 0 : dx;
		uint xmax = dx < 0 ? dx + pattern2.m_width : pattern1.m_width;
		uint ymin = dy < 0 ? 0 : dy;
		uint ymax = dy < 0 ? dy + pattern2.m_height : pattern1.m_width;

		// Iterate on every pixel contained in the intersection of the two patterns
		for (uint y = ymin; y < ymax; y++)
		{
			for (uint x = xmin; x < xmax; x++)
			{
				// Check if the color is the same in the two patterns in that pixel.
				if (pattern1.Get(y, x) != pattern2.Get(y - dy, x - dx))
				{
					return false;
				}
			}
		}
		return true;
	}

	//Precompute function IsPatternCompatibleWithThisPattern(pattern1, pattern2, dy, dx)
	//If it returns true then pattern2 is compatible with pattern1 in the direction defined by dy, dx
	//Add pattern2 to compatible[pattern1][direction]
	static std::vector<std::array<std::vector<unsigned>, 4>> GenerateCompatible(const std::vector<Array2D<Color>> &patterns)
	{
		std::vector<std::array<std::vector<unsigned>, 4>> compatible = std::vector<std::array<std::vector<unsigned>, 4>>(patterns.size());

		// Iterate on every dy, dx, pattern1 and pattern2
		for (unsigned pattern1 = 0; pattern1 < patterns.size(); pattern1++)
		{
			for (unsigned direction = 0; direction < 4; direction++)
			{
				for (unsigned pattern2 = 0; pattern2 < patterns.size(); pattern2++)
				{
					if (IsPatternCompatibleWithThisPattern(patterns[pattern1], patterns[pattern2], directions_y[direction], directions_x[direction]))
					{
						compatible[pattern1][direction].push_back(pattern2);
					}
				}
			}
		}

		return compatible;
	}

	//Transform a 2D array containing the patterns to a 2D array containing the pixels
	Array2D<Color> ToImage(const Array2D<unsigned> &output_patterns) const
	{
		Array2D<Color> output = Array2D<Color>(m_options.m_outHeight, m_options.m_outWidth);

		if (m_options.m_periodicOutput)
		{
			for (unsigned y = 0; y < m_options.GetWaveHeight(); y++)
			{
				for (unsigned x = 0; x < m_options.GetWaveWidth(); x++)
				{
					output.Get(y, x) = m_patterns[output_patterns.Get(y, x)].Get(0, 0);
				}
			}
		}
		else
		{

			for (unsigned y = 0; y < m_options.GetWaveHeight(); y++)
			{
				for (unsigned x = 0; x < m_options.GetWaveWidth(); x++)
				{
					output.Get(y, x) = m_patterns[output_patterns.Get(y, x)].Get(0, 0);
				}
			}

			for (unsigned y = 0; y < m_options.GetWaveHeight(); y++)
			{
				const Array2D<Color> &pattern = m_patterns[output_patterns.Get(y, m_options.GetWaveWidth() - 1)];

				for (unsigned dx = 1; dx < m_options.m_patternSize; dx++)
				{
					output.Get(y, m_options.GetWaveWidth() - 1 + dx) = pattern.Get(0, dx);
				}
			}

			for (unsigned x = 0; x < m_options.GetWaveWidth(); x++)
			{
				const Array2D<Color> &pattern = m_patterns[output_patterns.Get(m_options.GetWaveHeight() - 1, x)];
				for (unsigned dy = 1; dy < m_options.m_patternSize; dy++)
				{
					output.Get(m_options.GetWaveHeight() - 1 + dy, x) = pattern.Get(dy, 0);
				}
			}

			const Array2D<Color> &pattern = m_patterns[output_patterns.Get(m_options.GetWaveHeight() - 1, m_options.GetWaveWidth() - 1)];

			for (unsigned dy = 1; dy < m_options.m_patternSize; dy++)
			{
				for (unsigned dx = 1; dx < m_options.m_patternSize; dx++)
				{
					output.Get(m_options.GetWaveHeight() - 1 + dy, m_options.GetWaveWidth() - 1 + dx) = pattern.Get(dy, dx);
				}
			}
		}

		return output;
	}

public:
	//Constructor used by the user
	OverlappingWFC(const Array2D<Color> &input, const OverlappingWFCOptions &options, int seed)
		: OverlappingWFC(input, options, seed, GetPatterns(input, options))
	{

	}

	//Run the WFC algorithm, return the result if succeeded
	std::optional<Array2D<Color>> Run()
	{
		std::optional<Array2D<uint>> result = m_wfc.Run();
		if (result.has_value())
		{
			return ToImage(*result);
		}
		return std::nullopt;
	}

	const std::vector<Array2D<Color>>& GetPatterns()
	{
		return m_patterns;
	}
};
