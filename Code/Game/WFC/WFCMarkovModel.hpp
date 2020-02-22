#pragma once
#include "Game/WFC/WFCArray2D.hpp"
#include "Game/WFC/WFC.hpp"
#include "Game/WFC/WFCTile.hpp"
#include <vector>
#include <tuple>

//------------------------------------------------------------------------------------------------------------------------------
enum NeighborType
{
	RIGHT = 0,
	TOP,
	LEFT,
	BOTTOM
};

//------------------------------------------------------------------------------------------------------------------------------
struct MarkovWFCOptions
{
	uint m_tileSize;
	bool m_periodicOutput;
	uint m_height;
	uint m_width;
};

//------------------------------------------------------------------------------------------------------------------------------
//Class generating new image using Markov chain for neighboring information
template <typename T> class MarkovWFC
{
public:
	double m_initTime = 0;
	double m_neighborGenerationTime = 0;
	
private:

	//the distinct tiles
	std::vector< Tile<T> > m_tiles;

	//WFC options
	MarkovWFCOptions m_options;

	//The different inputs for Markov chain generation
	std::vector< Array2D<T> > m_inputs;

	//Id of oriented tiles to tile and orientation
	std::vector< std::pair<uint, uint> > m_idToOrientedTile;

	//Map tile and orientation to oriented tile id
	std::vector< std::vector<uint> > m_orientedTileIds;

	//Neighbors infered from reading samples
	std::vector< std::tuple<uint, uint, uint, uint> > m_inferedNeighbors;

	//The underlying generic WFC algorithm.
	WFC m_wfc;

	//The number of neighbor permutations
	int m_numPermutations;

	//------------------------------------------------------------------------------------------------------------------------------
	std::pair<uint, uint> FindTileAndMakeSymmetries(Array2D<T>& observedData)
	{
		//Check if any of the orientations of observedTile are part of m_tiles
		std::pair<uint, uint> tileIDtoOrientation = std::make_pair(UINT_MAX, UINT_MAX);

		bool foundTile = false;
		for (uint tileIndex = 0; tileIndex < (uint)m_tiles.size(); tileIndex++)
		{
			//NOTE!
			//The m_tiles will have all orientations, just store the tile index and orientation index to determine tile we are
			for (uint orientationIndex = 0; orientationIndex < NumPossibleOrientations(m_tiles[tileIndex].symmetry); orientationIndex++)
			{
				if (observedData == m_tiles[tileIndex].data[orientationIndex])
				{
					foundTile = true;
					//Since we found the correct tile, it is safe to assume the observed tile has the same symmetry and data

					tileIDtoOrientation.first = tileIndex;
					tileIDtoOrientation.second = orientationIndex;
				}

				if (foundTile)
				{
					break;
				}
			}

			if (foundTile)
			{
				break;
			}
		}

		return tileIDtoOrientation;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void PopulateNeighbor(Array2D<T>& tempTileArray, std::vector< std::pair <std::pair<uint, uint>, NeighborType> >& neighbors, NeighborType type)
	{
		std::pair<uint, uint> tileIDandOrientation = FindTileAndMakeSymmetries(tempTileArray);
		if (tileIDandOrientation.first != UINT_MAX)
		{
			neighbors.push_back(std::make_pair(tileIDandOrientation, type));
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	bool ValidateIndicesForSubArrayGet(uint yIndex, uint xIndex, unsigned int m_height, unsigned int m_width, unsigned int tileSize)
	{
		if (yIndex >= 0 && yIndex <= m_height - tileSize && xIndex >= 0 && xIndex <= m_width - tileSize)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	std::vector< std::pair <std::pair<uint, uint>, NeighborType> > FindNeighborsForTileAtPosition(uint xIndex, uint yIndex, uint tileSize, const Array2D<T>& inputImage)
	{
		std::vector< std::pair <std::pair<uint, uint> , NeighborType> > neighbors;
		std::pair<uint, uint> tileIDandOrientation = std::make_pair(UINT_MAX, UINT_MAX);
		Array2D<T> tempTileArray(tileSize, tileSize);
		bool validationResult = false;

		//Get the right neighbor
		validationResult = ValidateIndicesForSubArrayGet(yIndex, xIndex + tileSize, inputImage.m_height, inputImage.m_width, tileSize);
		if (validationResult)
		{
			tempTileArray = inputImage.GetSubArrayNonToric(yIndex, xIndex + tileSize, tileSize, tileSize);
			PopulateNeighbor(tempTileArray, neighbors, RIGHT);
		}

		//get the btoom neighbor
		validationResult = ValidateIndicesForSubArrayGet(yIndex + tileSize, xIndex, inputImage.m_height, inputImage.m_width, tileSize);
		if (validationResult)
		{
			tempTileArray = inputImage.GetSubArrayNonToric(yIndex + tileSize, xIndex, tileSize, tileSize);
			PopulateNeighbor(tempTileArray, neighbors, BOTTOM);
		}

		//get the left neighbor
		validationResult = ValidateIndicesForSubArrayGet(yIndex, xIndex - tileSize, inputImage.m_height, inputImage.m_width, tileSize);
		if (validationResult)
		{
			tempTileArray = inputImage.GetSubArrayNonToric(yIndex, xIndex - tileSize, tileSize, tileSize);
			PopulateNeighbor(tempTileArray, neighbors, LEFT);
		}

		//get the top neighbor
		validationResult = ValidateIndicesForSubArrayGet(yIndex - tileSize, xIndex, inputImage.m_height, inputImage.m_width, tileSize);
		if (validationResult)
		{
			tempTileArray = inputImage.GetSubArrayNonToric(yIndex - tileSize, xIndex, tileSize, tileSize);
			PopulateNeighbor(tempTileArray, neighbors, TOP);
		}

		return neighbors;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Get the rotated tile orientation depending on tile symmetry
	uint GetRotatedOrientationIDForObservedTile(const std::pair<uint, uint>& observedTileAndOrientation, uint numRotationsToPerform)
	{
		//Check the current tile's symmetry
		Symmetry symmetry = m_tiles[observedTileAndOrientation.first].symmetry;

		switch (symmetry)
		{
			case Symmetry::X:
			{
				//There is only 1 tile so just return 0
				return 0;
			}
			case Symmetry::I:
			case Symmetry::backslash:
			{
				//There are 2 possible tile orientations
				return (observedTileAndOrientation.second + numRotationsToPerform) % 2;
			}
			case Symmetry::T:
			case Symmetry::L:
			{
				//There are 4 possible tile orientations
				return (observedTileAndOrientation.second + numRotationsToPerform) % 4;
			}
			case Symmetry::P:
			{
				//Special case where we have 8 possible outcomes
				if (observedTileAndOrientation.second < 4)
				{
					//Normal case where we can add numRotations and mod by 4
					return (observedTileAndOrientation.second + numRotationsToPerform) % 4;
				}
				else
				{
					//Special case where orientation is above 4
					uint tempOrientation = observedTileAndOrientation.second;

					tempOrientation -= 4;
					tempOrientation += numRotationsToPerform;
					tempOrientation %= 4;

					tempOrientation += 4;

					return tempOrientation;
				}
			}
			default:
			{
				ERROR_AND_DIE("Couldn't find tile symmetry for the Markov problem to generate neighbors");
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Add the observed neighbor relationship to the vector of neighbor relationships
	void AddObservedNeighborToNeighborsVector(std::tuple<uint, uint, uint, uint>& neighborSet, std::vector<std::tuple<uint, uint, uint, uint> >& listToPopulate)
	{
		std::vector< std::tuple<uint, uint, uint, uint> >::iterator itr = std::find_if(std::begin(listToPopulate), std::end(listToPopulate), [&](const std::tuple<uint, uint, uint, uint>& checkValue)
			{
				return (std::get<0>(neighborSet) == std::get<0>(checkValue)
					&& std::get<1>(neighborSet) == std::get<1>(checkValue)
					&& std::get<2>(neighborSet) == std::get<2>(checkValue)
					&& std::get<3>(neighborSet) == std::get<3>(checkValue));
			});

		if (itr == listToPopulate.end())
		{
			listToPopulate.push_back(neighborSet);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Populate the neighbor information for the observed tile and observed neighbors
	void PopulateNeighborRelationshipsForObservedTile(std::pair<uint, uint>& observedIDtoOrientation, std::vector< std::pair <std::pair<uint, uint>, NeighborType> > neighbors, std::vector< std::tuple<uint, uint, uint, uint> >& tileIDOrientationtoNeighborSet)
	{
		std::tuple<uint, uint, uint, uint> tileIDOrientationtoNeighbor;
		uint observedOrientation;
		uint neighborOrientation;

		for (uint neighborIndex = 0; neighborIndex < neighbors.size(); neighborIndex++)
		{
			switch (neighbors[neighborIndex].second)
			{
				case RIGHT:
				{
					//Set the neighbor in it's current orientation
					tileIDOrientationtoNeighbor = std::make_tuple(observedIDtoOrientation.first, observedIDtoOrientation.second, neighbors[neighborIndex].first.first, neighbors[neighborIndex].first.second);
					
					AddObservedNeighborToNeighborsVector(tileIDOrientationtoNeighbor, tileIDOrientationtoNeighborSet);
					continue;
				}
				case TOP:
				{
					//Get rotated three times for both observed tile and neighbor tile
					observedOrientation = GetRotatedOrientationIDForObservedTile(observedIDtoOrientation, 3);
					neighborOrientation = GetRotatedOrientationIDForObservedTile(neighbors[neighborIndex].first, 3);

					tileIDOrientationtoNeighbor = std::make_tuple(observedIDtoOrientation.first, observedOrientation, neighbors[neighborIndex].first.first, neighborOrientation);
					AddObservedNeighborToNeighborsVector(tileIDOrientationtoNeighbor, tileIDOrientationtoNeighborSet);
					continue;
				}
				case LEFT:
				{
					//Get rotated twice for both observed tile and neighbor tile
					observedOrientation = GetRotatedOrientationIDForObservedTile(observedIDtoOrientation, 2);
					neighborOrientation = GetRotatedOrientationIDForObservedTile(neighbors[neighborIndex].first, 2);

					tileIDOrientationtoNeighbor = std::make_tuple(observedIDtoOrientation.first, observedOrientation, neighbors[neighborIndex].first.first, neighborOrientation);
					AddObservedNeighborToNeighborsVector(tileIDOrientationtoNeighbor, tileIDOrientationtoNeighborSet);
					continue;
				}
				case BOTTOM:
				{
					//Get rotated once for both observed tile and neighbor tile
					observedOrientation = GetRotatedOrientationIDForObservedTile(observedIDtoOrientation, 1);
					neighborOrientation = GetRotatedOrientationIDForObservedTile(neighbors[neighborIndex].first, 1);

					tileIDOrientationtoNeighbor = std::make_tuple(observedIDtoOrientation.first, observedOrientation, neighbors[neighborIndex].first.first, neighborOrientation);
					AddObservedNeighborToNeighborsVector(tileIDOrientationtoNeighbor, tileIDOrientationtoNeighborSet);
					continue;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Infer neighbors for the Markov WFC Problem
	std::vector<std::tuple<uint, uint, uint, uint>> InferNeighbors()
	{
		m_initTime = GetCurrentTimeSeconds();

		std::vector<std::tuple<uint, uint, uint, uint> > neighborSet;

		for (uint inputIndex = 0; inputIndex < m_inputs.size(); inputIndex++)
		{
			//Identify all pixels and turn them into a 2D array of tile ID and orientations
			uint xIndex = 0;
			uint yIndex = 0;

			while (xIndex != m_inputs[inputIndex].m_width && yIndex != m_inputs[inputIndex].m_height)
			{
				Array2D<T> observedData = Array2D<T>(m_options.m_tileSize, m_options.m_tileSize);
				Array2D<T> observedNeighborData = Array2D<T>(m_options.m_tileSize, m_options.m_tileSize);

				observedData = m_inputs[inputIndex].GetSubArray(yIndex, xIndex, m_options.m_tileSize, m_options.m_tileSize);

				//Find the tile in the set and get the correct Tile object with the correct symmetries and weights
				std::pair<uint, uint> observedIDtoOrientation = FindTileAndMakeSymmetries(observedData);
				if (observedIDtoOrientation.first == UINT_MAX)
				{
					ERROR_AND_DIE("ERROR! pattern observed does not correspond to any tile for Markov Problem");
				}

				//Find all the 4 neighbors for the tile
				std::vector< std::pair <std::pair<uint, uint>, NeighborType> > neighbors = FindNeighborsForTileAtPosition(xIndex, yIndex, m_options.m_tileSize, m_inputs[inputIndex]);

				//Generate relationships for the top, left, right and bottom tiles when generating neighbor information for the markov set
				PopulateNeighborRelationshipsForObservedTile(observedIDtoOrientation, neighbors, neighborSet);

				//Step ahead by tile size
				xIndex += m_options.m_tileSize;
				if (xIndex == m_inputs[inputIndex].m_width)
				{
					yIndex += m_options.m_tileSize;
					xIndex = 0;
				}
			}
		}

		m_neighborGenerationTime = GetCurrentTimeSeconds() - m_initTime;
		m_numPermutations = (int)neighborSet.size();

		return neighborSet;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Generate mapping from id to oriented tiles and vice versa
	static std::pair<std::vector<std::pair<uint, uint>>,
		std::vector<std::vector<uint>>>
		GenerateOrientedTileIDs(const std::vector<Tile<T>> &tiles) noexcept
	{
		std::vector<std::pair<uint, uint>> id_to_oriented_tile;
		std::vector<std::vector<uint>> oriented_tile_ids;

		uint id = 0;
		for (uint i = 0; i < tiles.size(); i++)
		{
			oriented_tile_ids.push_back({});
			for (uint j = 0; j < tiles[i].data.size(); j++)
			{
				id_to_oriented_tile.push_back({ i, j });
				oriented_tile_ids[i].push_back(id);
				id++;
			}
		}

		return { id_to_oriented_tile, oriented_tile_ids };
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Generate the propagator which will be used in the wfc algorithm
	static std::vector<std::array<std::vector<uint>, 4>> GeneratePropagator(
		const std::vector<std::tuple<uint, uint, uint, uint>>
		&neighbors,
		std::vector<Tile<T>> tiles,
		std::vector<std::pair<uint, uint>> id_to_oriented_tile,
		std::vector<std::vector<uint>> oriented_tile_ids)
	{
		size_t nb_oriented_tiles = id_to_oriented_tile.size();
		std::vector<std::array<std::vector<bool>, 4>> dense_propagator(
			nb_oriented_tiles, { std::vector<bool>(nb_oriented_tiles, false),
								std::vector<bool>(nb_oriented_tiles, false),
								std::vector<bool>(nb_oriented_tiles, false),
								std::vector<bool>(nb_oriented_tiles, false) });

		for (auto neighbor : neighbors)
		{
			uint tile1 = std::get<0>(neighbor);
			uint orientation1 = std::get<1>(neighbor);
			uint tile2 = std::get<2>(neighbor);
			uint orientation2 = std::get<3>(neighbor);
			std::vector<std::vector<uint>> action_map1 = Tile<T>::GenerateActionMap(tiles[tile1].symmetry);
			std::vector<std::vector<uint>> action_map2 = Tile<T>::GenerateActionMap(tiles[tile2].symmetry);

			auto add = [&](uint action, uint direction)
			{
				uint temp_orientation1 = action_map1[action][orientation1];
				uint temp_orientation2 = action_map2[action][orientation2];
				uint oriented_tile_id1 = oriented_tile_ids[tile1][temp_orientation1];
				uint oriented_tile_id2 = oriented_tile_ids[tile2][temp_orientation2];
				dense_propagator[oriented_tile_id1][direction][oriented_tile_id2] = true;
				direction = GetOppositeDirection(direction);
				dense_propagator[oriented_tile_id2][direction][oriented_tile_id1] = true;
			};

			add(0, 2);
			add(1, 0);
			add(2, 1);
			add(3, 3);
			add(4, 1);
			add(5, 3);
			add(6, 2);
			add(7, 0);
		}

		std::vector<std::array<std::vector<uint>, 4>> propagator(nb_oriented_tiles);

		for (size_t i = 0; i < nb_oriented_tiles; ++i)
		{
			for (size_t j = 0; j < nb_oriented_tiles; ++j)
			{
				for (size_t d = 0; d < 4; ++d)
				{
					if (dense_propagator[i][d][j])
					{
						propagator[i][d].push_back((uint)j);
					}
				}
			}
		}

		return propagator;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Get probability of presence of tiles
	static std::vector<double>	GetTilesWeight(const std::vector<Tile<T>> &tiles)
	{
		std::vector<double> frequencies;

		for (size_t i = 0; i < tiles.size(); ++i)
		{
			for (size_t j = 0; j < tiles[i].data.size(); ++j)
			{
				frequencies.push_back(tiles[i].weight / tiles[i].data.size());
			}
		}
		return frequencies;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//Translate generic WFC result into image
	Array2D<T> IDToTiling(Array2D<uint> ids)
	{
		uint size = m_tiles[0].data[0].m_height;

		Array2D<T> tiling(size * ids.m_height, size * ids.m_width);

		for (uint i = 0; i < ids.m_height; i++)
		{
			for (uint j = 0; j < ids.m_width; j++)
			{
				std::pair<uint, uint> oriented_tile = m_idToOrientedTile[ids.Get(i, j)];
				for (uint y = 0; y < size; y++)
				{
					for (uint x = 0; x < size; x++)
					{
						tiling.Get(i * size + y, j * size + x) = m_tiles[oriented_tile.first].data[oriented_tile.second].Get(y, x);
					}
				}
			}
		}
		return tiling;
	}


public:
	//Construct the TilingWFC Class to generate tiled image
	MarkovWFC(
		const std::vector<Tile<T>> &tiles,
		const std::vector<Array2D<T>> &inputs,
		const uint height, const uint width,
		const MarkovWFCOptions &options, int seed)
		: m_tiles(tiles),
		m_inputs(inputs),
		m_options(options),
		m_idToOrientedTile(GenerateOrientedTileIDs(tiles).first),
		m_orientedTileIds(GenerateOrientedTileIDs(tiles).second),
		m_inferedNeighbors(InferNeighbors()),
		m_wfc(m_options.m_periodicOutput, seed, GetTilesWeight(m_tiles), MarkovWFC::GeneratePropagator(m_inferedNeighbors, m_tiles, m_idToOrientedTile, m_orientedTileIds), width, height)
	{
	}

	//Run tiling WFC and return the result if succeeded
	std::optional<Array2D<T>> Run()
	{
		auto a = m_wfc.Run();
		if (a == std::nullopt)
		{
			return std::nullopt;
		}
		return IDToTiling(*a);
	}

	//Get Id of oriented tiles to tile and orientation
	const std::vector<std::pair<uint, uint>>& GetIDToOrientedTile() { return m_idToOrientedTile; }

	//Get orientation to oriented tile id
	const std::vector<std::vector<uint>>& GetOrientedTileIDs() { return m_orientedTileIds; }

	//Get num neighborhood permutations
	const int GetNumPermutations() { return m_numPermutations; }

	//------------------------------------------------------------------------------------------------------------------------------
	//Called after generating output of wfc. This will identify the neighborhood combinations used in the output
	int InferNeighborhoodCombinationsFromOutput(const Array2D<T>& output)
	{
		std::vector<std::tuple<uint, uint, uint, uint> > neighborSet;

		//Identify all pixels and turn them into a 2D array of tile ID and orientations
		uint xIndex = 0;
		uint yIndex = 0;

		while (xIndex != output.m_width && yIndex != output.m_height)
		{
			Array2D<T> observedData = Array2D<T>(m_options.m_tileSize, m_options.m_tileSize);
			Array2D<T> observedNeighborData = Array2D<T>(m_options.m_tileSize, m_options.m_tileSize);

			observedData = output.GetSubArray(yIndex, xIndex, m_options.m_tileSize, m_options.m_tileSize);

			//Find the tile in the set and get the correct Tile object with the correct symmetries and weights
			std::pair<uint, uint> observedIDtoOrientation = FindTileAndMakeSymmetries(observedData);
			if (observedIDtoOrientation.first == UINT_MAX)
			{
				ERROR_AND_DIE("ERROR! pattern observed does not correspond to any tile for Markov Problem");
			}

			//Find all the 4 neighbors for the tile
			std::vector< std::pair <std::pair<uint, uint>, NeighborType> > neighbors = FindNeighborsForTileAtPosition(xIndex, yIndex, m_options.m_tileSize, output);

			//Generate relationships for the top, left, right and bottom tiles when generating neighbor information for the markov set
			PopulateNeighborRelationshipsForObservedTile(observedIDtoOrientation, neighbors, neighborSet);

			//Step ahead by tile size
			xIndex += m_options.m_tileSize;
			if (xIndex == output.m_width)
			{
				yIndex += m_options.m_tileSize;
				xIndex = 0;
			}
		}

		return (int)neighborSet.size();
	}
};
