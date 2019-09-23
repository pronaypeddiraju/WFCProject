#pragma once
#include <random>
#include <optional>

#include "WFCArray2D.hpp"
#include "FastWFCPropagator.hpp"
#include "FastWFCWave.hpp"

//Class containing the generic WFC algorithm.
//------------------------------------------------------------------------------------------------------------------------------
class WFC 
{
private:
	//Random num generator
	std::minstd_rand gen;

	//The distribution of the patterns as given in input.
	const std::vector<double> patterns_frequencies;

	//The wave, indicating which patterns can be put in which cell.
	Wave wave;

	//The number of distinct patterns.
	const unsigned nb_patterns;

	//The propagator, used to propagate the information in the wave.
	Propagator propagator;

	
	//Transform the wave to a valid output (a 2d array of patterns that aren't in
	//contradiction). This function should be used only when all cell of the wave
	//are defined.
	Array2D<unsigned> WaveToOutput() const noexcept;

public:
	
	WFC(bool periodic_output, int seed, std::vector<double> patterns_frequencies,
		Propagator::PropagatorState propagator, unsigned wave_height,
		unsigned wave_width)
		noexcept;

	//Run WFC and return a result if we succeed
	std::optional<Array2D<unsigned>> Run() noexcept;

	//Return value of observe
	enum ObserveStatus {
		success,    // WFC has finished and has succeeded.
		failure,    // WFC has finished and failed.
		to_continue // WFC isn't finished.
	};

	//Define the value of the cell with lowest entropy.
	ObserveStatus Observe() noexcept;

	//Propagate information of the wave
	void Propagate() noexcept { propagator.Propagate(wave); }

	//Remove a pattern form cell i,j
	void RemoveWavePattern(unsigned i, unsigned j, unsigned pattern) noexcept 
	{
		if (wave.Get(i, j, pattern)) 
		{
			wave.Set(i, j, pattern, false);
			propagator.AddToPropagator(i, j, pattern);
		}
	}
};
