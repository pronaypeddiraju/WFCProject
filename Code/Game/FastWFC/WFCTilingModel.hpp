#pragma once
#include <unordered_map>
#include <vector>
#include "Game/FastWFC/WFCTile.hpp"
#include "Game/FastWFC/WFCArray2D.hpp"
#include "Game/FastWFC/WFC.hpp"

//------------------------------------------------------------------------------------------------------------------------------
//Return the number of possible distinct orientations for a tile
//Orientation is combination of rotations and reflections
//------------------------------------------------------------------------------------------------------------------------------
unsigned NumPossibleOrientations(const Symmetry &symmetry)
{
	switch (symmetry)
	{
	case Symmetry::X:
		return 1;
	case Symmetry::I:
	case Symmetry::backslash:
		return 2;
	case Symmetry::T:
	case Symmetry::L:
		return 4;
	default:
		return 8;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
//Options needed for tiling wfc
struct TilingWFCOptions
{
	bool periodic_output;
};

//------------------------------------------------------------------------------------------------------------------------------
//Class generating a new image with tiling WFC
template <typename T> class TilingWFC
{
private:
	//the distinct tiles
	std::vector<Tile<T>> m_tiles;

	//Id of oriented tiles to tile and orientation
	std::vector<std::pair<uint, uint>> m_idToOrientedTile;

	//Map tile and orientation to oriented tile id
	std::vector<std::vector<uint>> m_orientedTileIds;

	//WFC options
	TilingWFCOptions m_options;

	//The underlying generic WFC algorithm.
	WFC m_wfc;

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
				dense_propagator[oriented_tile_id1][direction][oriented_tile_id2] =	true;
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
	TilingWFC(
		const std::vector<Tile<T>> &tiles,
		const std::vector<std::tuple<uint, uint, uint, uint>>
		&neighbors,
		const uint height, const uint width,
		const TilingWFCOptions &options, int seed)
		: m_tiles(tiles),
		m_idToOrientedTile(GenerateOrientedTileIDs(tiles).first),
		m_orientedTileIds(GenerateOrientedTileIDs(tiles).second),
		m_options(options),
		m_wfc(options.periodic_output, seed, GetTilesWeight(tiles),
			GeneratePropagator(neighbors, tiles, m_idToOrientedTile,
				m_orientedTileIds),
			height, width) {}

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
};
