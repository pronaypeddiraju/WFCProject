#include "Game/WFC/WFC.hpp"
#include <limits>

//------------------------------------------------------------------------------------------------------------------------------
namespace
{
	//Normalize a vector so the sum of its elements is equal to 1.0f
	std::vector<double>& normalize(std::vector<double>& vector)
	{
		double sumWeights = 0.0;
		for (double weight : vector)
		{
			sumWeights += weight;
		}

		double invSumWeights = 1.0 / sumWeights;
		for (double& weight : vector)
		{
			weight *= invSumWeights;
		}

		return vector;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
Array2D<uint> WFC::WaveToOutput() 
{
	Array2D<uint> outputPatterns(m_wave.height, m_wave.width);
	for (uint i = 0; i < m_wave.size; i++)
	{
		for (uint k = 0; k < m_numPatterns; k++)
		{
			if (m_wave.Get(i, k))
			{
				outputPatterns.m_data[i] = k;
				m_cachedOutputPatterns.m_data[i] = k;
			}
		}
	}

	return outputPatterns;
}

//------------------------------------------------------------------------------------------------------------------------------
WFC::WFC(bool periodicOutputs, int seed, std::vector<double> patternsFrequencies, Propagator::PropagatorState propagator, uint waveHeight, uint waveWidth) 
	: m_randomGenerator(seed), m_patternFrequencies(normalize(patternsFrequencies)),
	m_wave(waveHeight, waveWidth, patternsFrequencies),
	m_numPatterns((uint)propagator.size()),
	m_propagator(m_wave.height, m_wave.width, periodicOutputs, propagator),
	m_cachedOutputPatterns(waveHeight, waveWidth)
{}

//------------------------------------------------------------------------------------------------------------------------------
std::optional<Array2D<uint>> WFC::Run() 
{
	while (true)
	{

		// Define the value of an undefined cell.
		ObserveStatus result = Observe();

		// Check if the algorithm has terminated.
		if (result == FAILURE) {
			return std::nullopt;
		}
		else if (result == SUCCESS) {
			return WaveToOutput();
		}

		// Propagate the information.
		m_propagator.Propagate(m_wave);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
WFC::ObserveStatus WFC::Observe() 
{
	// Get the cell with lowest entropy.
	int argmin = m_wave.GetMinEntropy(m_randomGenerator);

	// If there is a contradiction, the algorithm has failed.
	if (argmin == -2)
	{
		return FAILURE;
	}

	// If the lowest entropy is 0, then the algorithm has succeeded and
	// finished.
	if (argmin == -1)
	{
		WaveToOutput();
		return SUCCESS;
	}

	// Choose an element according to the pattern distribution
	double s = 0;
	for (uint k = 0; k < m_numPatterns; k++)
	{
		s += m_wave.Get(argmin, k) ? m_patternFrequencies[k] : 0;
	}

	std::uniform_real_distribution<> dis(0, s);
	double random_value = dis(m_randomGenerator);
	uint chosen_value = m_numPatterns - 1;

	for (uint k = 0; k < m_numPatterns; k++)
	{
		random_value -= m_wave.Get(argmin, k) ? m_patternFrequencies[k] : 0;
		if (random_value <= 0)
		{
			chosen_value = k;
			break;
		}
	}

	// And define the cell with the pattern.
	for (uint k = 0; k < m_numPatterns; k++)
	{
		if (m_wave.Get(argmin, k) != (k == chosen_value))
		{
			m_propagator.AddToPropagator(argmin / m_wave.width, argmin % m_wave.width, k);
			m_wave.Set(argmin, k, false);
		}
	}

	return TO_CONTINUE;
}
