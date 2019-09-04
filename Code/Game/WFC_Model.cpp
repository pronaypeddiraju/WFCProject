#include "Game/WFC_Model.hpp"
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Model::Model(int width, int height)
{
	m_modelDimensions = IntVec2(width, height);
	m_modelTotalBlocks = width * height;
}

//------------------------------------------------------------------------------------------------------------------------------
void Model::Startup()
{
	//Initialize wave and compatibility data
	m_waveArray = Array2D<bool>(IntVec2(m_modelTotalBlocks, m_numWeights), false);
	m_compatibleBlocks = Array2D<std::vector<int>>(IntVec2(m_modelTotalBlocks, m_numWeights), std::vector<int>(0, 0));
	for (int i = 0; i < m_waveArray.GetWidth(); i++)
	{
		//Set 4 ints in the m_compatibleBlocks array for values
		for (int j = 0; j < m_numWeights; j++)
		{
			m_compatibleBlocks.Set(IntVec2(i, j), {0,0,0,0});
		}
	}

	//Initializing all weight log weights
	for (int weightIndex = 0; weightIndex < m_numWeights; weightIndex++)
	{
		m_weightLogWeights[weightIndex] = m_weights[weightIndex] * log(m_weights[weightIndex]);
		m_sumOfWeights += m_weights[weightIndex];
		m_sumOfWeightLogWeights += m_weightLogWeights[weightIndex];
	}

	m_startingEntropy = log(m_sumOfWeights) - (m_sumOfWeightLogWeights / m_sumOfWeights);

	m_sumOfOnes = std::vector<int>(m_modelTotalBlocks, 0);
	m_sumsOfWeights = std::vector<double>(m_modelTotalBlocks, 0.0);
	m_sumsOfWeightLogWeights = std::vector<double>(m_modelTotalBlocks, 0.0);
	m_entropies = std::vector<double>(m_modelTotalBlocks, 0.0);
}

//------------------------------------------------------------------------------------------------------------------------------
bool Model::Observe()
{
	double min = 1E+3;
	int argmin = -1;

	for (int i = 0; i < m_waveArray.GetWidth(); i++)
	{
		TODO("Make OnBoundary");
		if (OnBoundary(i % m_modelDimensions.x, i / m_modelDimensions.x))
		{
			continue;
		}

		int amount = m_sumOfOnes[i];
		if (amount == 0)
		{
			return false;
		}


		double entropy = m_entropies[i];
		if (amount > 1 && entropy <= min)
		{
			double noise = 1E-6 * g_RNG->GetRandomFloatInRange(0.f, FLT_MAX);
			if (entropy + noise < min)
			{
				min = entropy + noise;
				argmin = i;
			}
		}
	}

	if (argmin == -1)
	{
		for (int blockIndex = 0; blockIndex < m_waveArray.GetWidth(); blockIndex++)
		{
			for (int weightIndex = 0; weightIndex < m_numWeights; weightIndex++)
			{
				if (m_waveArray.ContainsCell(IntVec2(blockIndex, weightIndex)))
				{
					m_observedBlocks[blockIndex] = weightIndex;
					break;
				}
			}
		}
		
		return true;
	}


	std::vector<double> distribution;
	distribution.resize(sizeof(double) * m_numWeights);
	for (int weightIndex = 0; weightIndex < m_numWeights; weightIndex++)
	{
		distribution[weightIndex] = m_waveArray.ContainsCell(IntVec2(argmin, weightIndex)) ? m_weights[weightIndex] : 0;
	}

	int randomInt = (int)distribution[g_RNG->GetRandomIntInRange(0, (int)distribution.size() - 1)];

	std::vector<bool> waveStates(argmin, false);
	for (int weightIndex = 0; weightIndex < m_numWeights; weightIndex++)
	{
		if (waveStates[weightIndex] != (weightIndex == randomInt))
		{
			Ban(argmin, weightIndex);
		}
	}
	
	return false;
}

//------------------------------------------------------------------------------------------------------------------------------
void Model::Ban(int waveIndex, int weightIndex)
{
	m_waveArray.Set(IntVec2(waveIndex, weightIndex), false);

	std::vector<int> comp = m_compatibleBlocks.Get(IntVec2(waveIndex, weightIndex));
	for (int d = 0; d < 4; d++)
	{
		comp[d] = 0;
	}
	
	TODO("Create the tuple and push it in the stack");
	//m_stack.push()
	//stacksize++;

	m_sumOfOnes[waveIndex] -= 1;
	m_sumsOfWeights[waveIndex] -= m_weights[weightIndex];
	m_sumsOfWeightLogWeights[waveIndex] -= m_weightLogWeights[weightIndex];

	double sum = m_sumsOfWeights[waveIndex];
	m_entropies[waveIndex] = log(sum) - m_sumsOfWeightLogWeights[waveIndex] / sum;
}

//------------------------------------------------------------------------------------------------------------------------------
void Model::Propagate()
{

}
