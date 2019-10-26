#include "Game/FastWFC/WFCWave.hpp"

#include <limits>

//------------------------------------------------------------------------------------------------------------------------------
namespace
{
	//Return distribution * log(distribution)
	std::vector<double> GetPlogP(const std::vector<double> &distribution) noexcept
	{
		std::vector<double> plogp;
		for (unsigned i = 0; i < distribution.size(); i++)
		{
			plogp.push_back(distribution[i] * log(distribution[i]));
		}
		return plogp;
	}

	//Return min(v)/2
	double GetHalfOfMinAbsolute(const std::vector<double> &v) noexcept
	{
		double minHalfOfAbsolute = std::numeric_limits<double>::infinity();
		for (unsigned i = 0; i < v.size(); i++)
		{
			minHalfOfAbsolute = std::min(minHalfOfAbsolute, std::abs(v[i] / 2.0));
		}
		return minHalfOfAbsolute;
	}

} // namespace

//------------------------------------------------------------------------------------------------------------------------------
Wave::Wave(unsigned height, unsigned width,
	const std::vector<double> &patterns_frequencies) noexcept
	: m_patternsFrequencies(patterns_frequencies),
	m_plogpPatternFrequencies(GetPlogP(patterns_frequencies)),
	min_abs_half_plogp(GetHalfOfMinAbsolute(m_plogpPatternFrequencies)),
	m_isImpossible(false), m_nbPatterns((unsigned)patterns_frequencies.size()),
	m_data(width * height, m_nbPatterns, 1), width(width), height(height),
	size(height * width)
{
	// Initialize the memoisation of entropy.
	double base_entropy = 0;
	double base_s = 0;
	for (unsigned i = 0; i < m_nbPatterns; i++)
	{
		base_entropy += m_plogpPatternFrequencies[i];
		base_s += patterns_frequencies[i];
	}

	double log_base_s = log(base_s);
	double entropy_base = log_base_s - base_entropy / base_s;
	memoisation.plogp_sum = std::vector<double>(width * height, base_entropy);
	memoisation.sum = std::vector<double>(width * height, base_s);
	memoisation.log_sum = std::vector<double>(width * height, log_base_s);
	memoisation.nb_patterns = std::vector<unsigned>(width * height, m_nbPatterns);
	memoisation.entropy = std::vector<double>(width * height, entropy_base);
}

//------------------------------------------------------------------------------------------------------------------------------
void Wave::Set(unsigned index, unsigned pattern, bool value) noexcept
{
	bool old_value = m_data.Get(index, pattern);
	// If the value isn't changed, nothing needs to be done.
	if (old_value == value)
	{
		return;
	}
	// Otherwise, the memoisation should be updated.
	m_data.Get(index, pattern) = value;
	memoisation.plogp_sum[index] -= m_plogpPatternFrequencies[pattern];
	memoisation.sum[index] -= m_patternsFrequencies[pattern];
	memoisation.log_sum[index] = log(memoisation.sum[index]);
	memoisation.nb_patterns[index]--;
	memoisation.entropy[index] = memoisation.log_sum[index] - memoisation.plogp_sum[index] / memoisation.sum[index];
	// If there is no patterns possible in the cell, then there is a
	// contradiction.
	if (memoisation.nb_patterns[index] == 0)
	{
		m_isImpossible = true;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
int Wave::GetMinEntropy(std::minstd_rand &gen) const noexcept
{
	if (m_isImpossible)
	{
		return -2;
	}

	std::uniform_real_distribution<> dis(0, min_abs_half_plogp);

	// The minimum entropy (plus a small noise)
	double min = std::numeric_limits<double>::infinity();
	int argmin = -1;

	for (unsigned i = 0; i < size; i++)
	{

		// If the cell is decided, we do not compute the entropy (which is equal
		// to 0).
		double numPatterns = memoisation.nb_patterns[i];
		if (numPatterns == 1)
		{
			continue;
		}

		// Otherwise, we take the memoised entropy.
		double entropy = memoisation.entropy[i];

		// We first check if the entropy is less than the minimum.
		// This is important to reduce noise computation (which is not
		// negligible).
		if (entropy <= min)
		{

			// Then, we add noise to decide randomly which will be chosen.
			// noise is smaller than the smallest p * log(p), so the minimum entropy
			// will always be chosen.
			double noise = dis(gen);
			if (entropy + noise < min)
			{
				min = entropy + noise;
				argmin = i;
			}
		}
	}

	return argmin;
}
