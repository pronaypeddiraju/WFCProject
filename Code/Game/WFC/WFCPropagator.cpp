#include "Game/WFC/WFCPropagator.hpp"
#include "Game/WFC/WFC.hpp"

typedef unsigned int uint;

//------------------------------------------------------------------------------------------------------------------------------
void Propagator::InitializeCompatible()
{
	std::array<int, 4> value;
	// We compute the number of pattern compatible in all directions.
	for (uint y = 0; y < m_waveHeight; y++)
	{
		for (uint x = 0; x < m_waveWidth; x++)
		{
			for (uint pattern = 0; pattern < m_patternsSize; pattern++)
			{
				for (int direction = 0; direction < 4; direction++)
				{
					value[direction] = (int)m_propagator_state[pattern][GetOppositeDirection(direction)].size();
				}
				compatible.Get(y, x, pattern) = value;
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Propagator::Propagate(Wave &wave)
{
	// We propagate every element while there is elements to propagate.
	while (propagating.size() != 0)
	{
		// The cell and pattern that has been set to false.
		uint y1, x1, pattern;
		std::tie(y1, x1, pattern) = propagating.back();
		propagating.pop_back();

		// We propagate the information in all 4 directions.
		for (uint direction = 0; direction < 4; direction++)
		{
			// We get the next cell in the direction direction.
			int dx = directions_x[direction];
			int dy = directions_y[direction];
			int x2, y2;

			if (periodic_output)
			{
				x2 = ((int)x1 + dx + (int)wave.width) % wave.width;
				y2 = ((int)y1 + dy + (int)wave.height) % wave.height;
			}
			else
			{
				x2 = x1 + dx;
				y2 = y1 + dy;
				if (x2 < 0 || x2 >= (int)wave.width)
				{
					continue;
				}
				if (y2 < 0 || y2 >= (int)wave.height)
				{
					continue;
				}
			}

			// The index of the second cell, and the patterns compatible
			uint i2 = x2 + y2 * wave.width;
			const std::vector<uint> &patterns = m_propagator_state[pattern][direction];

			// For every pattern that could be placed in that cell without being in
			// contradiction with pattern1
			for (auto it = patterns.begin(), it_end = patterns.end(); it < it_end; ++it)
			{

				// We decrease the number of compatible patterns in the opposite
				// direction If the pattern was discarded from the wave, the element
				// is still negative, which is not a problem
				std::array<int, 4> &value = compatible.Get(y2, x2, *it);
				value[direction]--;

				// If the element was set to 0 with this operation, we need to remove
				// the pattern from the wave, and propagate the information
				if (value[direction] == 0)
				{
					AddToPropagator(y2, x2, *it);
					wave.Set(i2, *it, false);
				}
			}
		}
	}
}
