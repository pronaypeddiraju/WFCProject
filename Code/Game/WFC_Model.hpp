#pragma once
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Math/Array2D.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <stack>
#include <tuple>

//------------------------------------------------------------------------------------------------------------------------------
class Model
{
public:
	Model(int width, int height);

	void	Startup();
	bool	Observe();
	void	Ban(int waveIndex, int weightIndex);
	void	Propagate();

	virtual bool	OnBoundary(int x, int y) = 0;

private:
	//wave
	Array2D<bool>	m_waveArray;		
	uint			m_waveRowSize = 0;
	uint			m_waveColSize = 0;
	
	//propagation
	Array2D<std::vector<int>>	m_propagatorBlocks;
	Array2D<std::vector<int>>	m_compatibleBlocks;
	std::vector<int>			m_observedBlocks;
	int							m_numPropagationBlocks = 0;

	//Stack
	//std::map<int, int>	m_stack;
	//int m_stackSize = 0;
	//Figure a good way to do tuples here

	//Generator Variables
	int					m_randomSeed = 0;
	IntVec2				m_modelDimensions = IntVec2::ZERO;
	int					m_modelTotalBlocks = 0;
	int					m_numWeights = 0;
	bool				m_isPeriodic = false;
	std::vector<double>	m_weights;
	std::vector<double>	m_weightLogWeights;

	std::vector<int>	m_sumOfOnes;
	double				m_sumOfWeights;
	double				m_sumOfWeightLogWeights;
	double				m_startingEntropy;

	std::vector<double>	m_sumsOfWeights;
	std::vector<double>	m_sumsOfWeightLogWeights;
	std::vector<double>	m_entropies;
};