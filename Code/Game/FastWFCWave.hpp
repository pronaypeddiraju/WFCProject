#pragma once
#include "WFCArray2D.hpp"
#include <random>
#include <vector>

/**
 * Struct containing the values needed to compute the entropy of all the cells.
 * This struct is updated every time the wave is changed.
 * p'(pattern) is equal to patterns_frequencies[pattern] if wave.get(cell,
 * pattern) is set to true, otherwise 0.
 */
struct EntropyMemoisation {
	std::vector<double> plogp_sum; // The sum of p'(pattern) * log(p'(pattern)).
	std::vector<double> sum;       // The sum of p'(pattern).
	std::vector<double> log_sum;   // The log of sum.
	std::vector<unsigned> nb_patterns; // The number of patterns present
	std::vector<double> entropy;       // The entropy of the cell.
};

//Contains the pattern possibilities in every cell. Also contains info about cell entropy
class Wave 
{
	private:
		//Patterns frequencies p given to wfc
		const std::vector<double> m_patternsFrequencies;

		//Precomputation of p * log(p)
		const std::vector<double> m_plogpPatternFrequencies;

		//Precomputed min(p*log(p))/2
		//This is used to define the maximum value of the noise
		const double min_abs_half_plogp;

		//memoisation of important values for computation of entropy
		EntropyMemoisation memoisation;

		//This is set to true if there is a contradiction in the wave
		//(i.e: all elements are set to false in a cell)
		bool m_isImpossible;

		//The number of distinct patterns
		const unsigned m_nbPatterns;

		//The actual wave. m_data.get(index, pattern) is 0 if the pattern can be placed
		//in the cell index
		Array2D<uint8_t> m_data;

	public:
		//size of the wave
		const unsigned width;
		const unsigned height;
		const unsigned size;

		//Initialize the wave with every cell being able to have every pattern
		Wave(unsigned height, unsigned width, const std::vector<double> &patterns_frequencies) noexcept;

		//If the pattern can be placed in cell index, return true
		bool Get(unsigned index, unsigned pattern) const noexcept 
		{
			return m_data.get(index, pattern);
		}

		//Return true if the pattern can be placed in cell (i,j)
		bool Get(unsigned i, unsigned j, unsigned pattern) const noexcept 
		{
			return Get(i * width + j, pattern);
		}

		//Set the value of the pattern in cell index
		void Set(unsigned index, unsigned pattern, bool value) noexcept;

		//Set the value of the pattern in cell (i,j)
		void Set(unsigned i, unsigned j, unsigned pattern, bool value) noexcept 
		{
			Set(i * width + j, pattern, value);
		}

		//Return index of cell with lowest entropy different of 0
		//If there is a contradiction in the wave return -2
		//If every cell is decided, return -1
		int GetMinEntropy(std::minstd_rand &gen) const noexcept;
};