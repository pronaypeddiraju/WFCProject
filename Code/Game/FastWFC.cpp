#include "Game/FastWFC.hpp"
#include <limits>

namespace 
{
	//Normalize a vector so the sum of its elements is equal to 1.0f
	std::vector<double>& normalize(std::vector<double>& v) 
	{
		double sum_weights = 0.0;
		for (double weight : v) 
		{
			sum_weights += weight;
		}

		double inv_sum_weights = 1.0 / sum_weights;
		for (double& weight : v) 
		{
			weight *= inv_sum_weights;
		}

		return v;
	}
}


Array2D<unsigned> WFC::WaveToOutput() const noexcept 
{
	Array2D<unsigned> output_patterns(wave.height, wave.width);
	for (unsigned i = 0; i < wave.size; i++) 
	{
		for (unsigned k = 0; k < nb_patterns; k++) 
		{
			if (wave.Get(i, k)) 
			{
				output_patterns.m_data[i] = k;
			}
		}
	}
	return output_patterns;
}

WFC::WFC(bool periodic_output, int seed, std::vector<double> patterns_frequencies, Propagator::PropagatorState propagator, unsigned wave_height, unsigned wave_width) noexcept
	: gen(seed), patterns_frequencies(normalize(patterns_frequencies)),
	wave(wave_height, wave_width, patterns_frequencies),
	nb_patterns((unsigned)propagator.size()),
	propagator(wave.height, wave.width, periodic_output, propagator) 
{

}

std::optional<Array2D<unsigned>> WFC::Run() noexcept 
{
	while (true) 
	{

		// Define the value of an undefined cell.
		ObserveStatus result = Observe();

		// Check if the algorithm has terminated.
		if (result == failure) {
			return std::nullopt;
		}
		else if (result == success) {
			return WaveToOutput();
		}

		// Propagate the information.
		propagator.Propagate(wave);
	}
}


WFC::ObserveStatus WFC::Observe() noexcept 
{
	// Get the cell with lowest entropy.
	int argmin = wave.GetMinEntropy(gen);

	// If there is a contradiction, the algorithm has failed.
	if (argmin == -2) 
	{
		return failure;
	}

	// If the lowest entropy is 0, then the algorithm has succeeded and
	// finished.
	if (argmin == -1) 
	{
		WaveToOutput();
		return success;
	}

	// Choose an element according to the pattern distribution
	double s = 0;
	for (unsigned k = 0; k < nb_patterns; k++) 
	{
		s += wave.Get(argmin, k) ? patterns_frequencies[k] : 0;
	}

	std::uniform_real_distribution<> dis(0, s);
	double random_value = dis(gen);
	unsigned chosen_value = nb_patterns - 1;

	for (unsigned k = 0; k < nb_patterns; k++) 
	{
		random_value -= wave.Get(argmin, k) ? patterns_frequencies[k] : 0;
		if (random_value <= 0) 
		{
			chosen_value = k;
			break;
		}
	}

	// And define the cell with the pattern.
	for (unsigned k = 0; k < nb_patterns; k++) 
	{
		if (wave.Get(argmin, k) != (k == chosen_value)) 
		{
			propagator.AddToPropagator(argmin / wave.width, argmin % wave.width, k);
			wave.Set(argmin, k, false);
		}
	}

	return to_continue;
}
