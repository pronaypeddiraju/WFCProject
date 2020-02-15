#pragma once
#include "Game/WFC/WFCDirection.hpp"
#include "Game/WFC/WFCArray3D.hpp"
#include <tuple>
#include <vector>
#include <array>

class Wave;

//------------------------------------------------------------------------------------------------------------------------------
//Class to propagate pattern information from the input wave
//------------------------------------------------------------------------------------------------------------------------------
class Propagator
{
public:
	using PropagatorState = std::vector<std::array<std::vector<unsigned int>, 4>>;


	const unsigned int m_patternsSize;

	//Propagator[pattern1][direction] contains all the patterns that can
	//be placed in next to pattern1 in the direction 'direction'.
	PropagatorState m_propagator_state;

	const unsigned m_waveWidth;
	const unsigned m_waveHeight;

	//True if wave and output are toric
	const bool periodic_output;

	//All the tuples (y, x, pattern) that should be propagated.
	//The tuple should be propagated when wave.get(y, x, pattern) is set to
	//false.
	std::vector<std::tuple<unsigned, unsigned, unsigned>> propagating;

	//compatible.get(y, x, pattern)[direction] contains the number of patterns
	//present in the wave that can be placed in the cell next to(y, x) in the
	//opposite direction of direction without being in contradiction with pattern
	//placed in(y, x).If wave.get(y, x, pattern) is set to false, then
	//compatible.get(y, x, pattern) has every element negative or null
	Array3D<std::array<int, 4>> compatible;

private:
	//compute compatible patterns in all directions
	void InitializeCompatible();

public:

	Propagator(unsigned wave_height, unsigned wave_width, bool periodic_output, PropagatorState propagator_state)
		: m_patternsSize((unsigned)propagator_state.size()),
		m_propagator_state(propagator_state), m_waveWidth(wave_width),
		m_waveHeight(wave_height), periodic_output(periodic_output),
		compatible(wave_height, wave_width, m_patternsSize)
	{
		InitializeCompatible();
	}

	//Add an element to the propagator
	//Called when wave.Get(y, x, pattern) is set to false
	void AddToPropagator(unsigned y, unsigned x, unsigned pattern)
	{
		// All the direction are set to 0, since the pattern cannot be set in (y,x).
		std::array<int, 4> temp = {};
		compatible.Get(y, x, pattern) = temp;
		propagating.emplace_back(y, x, pattern);
	}

	//Propagate information given from AddToPropagator
	void Propagate(Wave &wave);
};
