#pragma once
#include "Game/FastWFC/WFCArray2D.hpp"
#include "Game/FastWFC/WFC.hpp"
#include "Game/FastWFC/WFCTilingModel.hpp"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------
class MarkovWFCOptions
{

};

//------------------------------------------------------------------------------------------------------------------------------
//Class generating new image using Markov chain for neighboring information
template <typename T> class MarkovWFC
{
private:

	//the distinct tiles
	std::vector<Tile<T>> tiles;

	//Id of oriented tiles to tile and orientation
	std::vector<std::pair<unsigned, unsigned>> id_to_oriented_tile;

	//Map tile and orientation to oriented tile id
	std::vector<std::vector<unsigned>> oriented_tile_ids;

	//WFC options
	MarkovWFCOptions options;

	//The underlying generic WFC algorithm.
	WFC wfc;

public:
	//Construct the TilingWFC Class to generate tiled image
	MarkovWFC(
		const std::vector<Tile<T>> &tiles,
		const std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>>
		&neighbors,
		const unsigned height, const unsigned width,
		const MarkovWFCOptions &options, int seed)
		: tiles(tiles),
		id_to_oriented_tile(GenerateOrientedTileIDs(tiles).first),
		oriented_tile_ids(GenerateOrientedTileIDs(tiles).second),
		options(options),
		wfc(options.periodic_output, seed, GetTilesWeight(tiles),
			GeneratePropagator(neighbors, tiles, id_to_oriented_tile,
				oriented_tile_ids),
			height, width) {}

	//Run tiling WFC and return the result if succeeded
	std::optional<Array2D<T>> Run()
	{
		auto a = wfc.Run();
		if (a == std::nullopt)
		{
			return std::nullopt;
		}
		return IDToTiling(*a);
	}

	//Get Id of oriented tiles to tile and orientation
	const std::vector<std::pair<uint, uint>>& GetIDToOrientedTile() { return id_to_oriented_tile; }

	//Get orientation to oriented tile id
	const std::vector<std::vector<uint>>& GetOrientedTileIDs() { return oriented_tile_ids; }
};
