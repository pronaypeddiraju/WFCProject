#pragma once
#include <random>
#include <optional>

#include "Game/WFC/WFCArray2D.hpp"
#include "Game/WFC/WFCPropagator.hpp"
#include "Game/WFC/WFCWave.hpp"

typedef unsigned int uint;

//------------------------------------------------------------------------------------------------------------------------------
//Class containing the generic WFC algorithm.
class WFC
{
private:
	//Random num generator
	std::minstd_rand m_randomGenerator;

	//The distribution of the patterns as given in input.
	const std::vector<double> m_patternFrequencies;

	//The wave, indicating which patterns can be put in which cell.
	Wave m_wave;

	//The number of distinct patterns.
	const uint m_numPatterns;

	//Cached output patterns from WFC
	Array2D<uint> m_cachedOutputPatterns;

	//Transform the wave to a valid output (a 2d array of patterns that aren't in
	//contradiction). This function should be used only when all cell of the wave
	//are defined.
	Array2D<uint> WaveToOutput();

public:

	//The propagator, used to propagate the information in the wave.
	Propagator m_propagator;

	WFC(bool periodicOutput, int seed, std::vector<double> patternFrequencies,
		Propagator::PropagatorState propagator, uint waveHeight,
		uint waveWidth);

	//Run WFC and return a result if we succeed
	std::optional<Array2D<uint>> Run();

	//Return value of observe
	enum ObserveStatus 
	{
		SUCCESS,    // WFC has finished and has succeeded.
		FAILURE,    // WFC has finished and failed.
		TO_CONTINUE // WFC isn't finished.
	};

	//Define the value of the cell with lowest entropy.
	ObserveStatus Observe();

	//Propagate information of the wave
	void Propagate() { m_propagator.Propagate(m_wave); }

	//Remove a pattern form cell i,j
	void RemoveWavePattern(uint i, uint j, uint pattern)
	{
		if (m_wave.Get(i, j, pattern))
		{
			m_wave.Set(i, j, pattern, false);
			m_propagator.AddToPropagator(i, j, pattern);
		}
	}
};
