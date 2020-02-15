#pragma once

//The distinct symmetries of a tile
//Represents how the tile should behace when it is rotated or reflected
enum class Symmetry { X, T, I, L, backslash, P };

//------------------------------------------------------------------------------------------------------------------------------
//A tile that can be placed on the board.
template <typename T> struct Tile
{
	std::vector<Array2D<T>> data; // The different orientations of the tile
	Symmetry symmetry;            // The symmetry of the tile
	double weight;					// Its weight on the distribution of presence of tiles
	std::string tileName;

	//Generate map associating an orientation id to the orientation
	//id obtained when rotating tile by 90 degrees anticlockwise
	static std::vector<uint> GenerateRotationMap(const Symmetry &symmetry)
	{
		switch (symmetry)
		{
		case Symmetry::X:
			return { 0 };
		case Symmetry::I:
		case Symmetry::backslash:
			return { 1, 0 };
		case Symmetry::T:
		case Symmetry::L:
			return { 1, 2, 3, 0 };
		case Symmetry::P:
		default:
			return { 1, 2, 3, 0, 5, 6, 7, 4 };
		}
	}

	//Generate map associating orientation id to the orientation
	//id obtained when reflecting the tile along x axis
	static std::vector<uint> GenerateReflectionMap(const Symmetry &symmetry)
	{
		switch (symmetry)
		{
		case Symmetry::X:
			return { 0 };
		case Symmetry::I:
			return { 0, 1 };
		case Symmetry::backslash:
			return { 1, 0 };
		case Symmetry::T:
			return { 0, 3, 2, 1 };
		case Symmetry::L:
			return { 1, 0, 3, 2 };
		case Symmetry::P:
		default:
			return { 4, 7, 6, 5, 0, 3, 2, 1 };
		}
	}

	//Generate the map associating orientation id and an action to the resulting
	//orientation id.
	//Actions 0,1,2,3 are 0°,90°,180° and 270° degree anticlockwise rotations
	//Actions 4,5,6,7 are 0,1,2,3 preceded by a reflection on the x axis
	static std::vector<std::vector<uint>> GenerateActionMap(const Symmetry &symmetry)
	{
		std::vector<uint> rotation_map = GenerateRotationMap(symmetry);
		std::vector<uint> reflection_map = GenerateReflectionMap(symmetry);
		size_t size = rotation_map.size();
		std::vector<std::vector<uint>> action_map(8, std::vector<uint>(size));

		for (size_t i = 0; i < size; ++i)
		{
			action_map[0][i] = (uint)i;
		}

		for (size_t a = 1; a < 4; ++a)
		{
			for (size_t i = 0; i < size; ++i)
			{
				action_map[a][i] = rotation_map[action_map[a - 1][i]];
			}
		}

		for (size_t i = 0; i < size; ++i)
		{
			action_map[4][i] = reflection_map[action_map[0][i]];
		}

		for (size_t a = 5; a < 8; ++a)
		{
			for (size_t i = 0; i < size; ++i)
			{
				action_map[a][i] = rotation_map[action_map[a - 1][i]];
			}
		}
		return action_map;
	}

	//Generate all distincts rotations of a 2D array given its symmetries;
	static std::vector<Array2D<T>> GenerateOriented(Array2D<T> data, Symmetry symmetry)
	{
		std::vector<Array2D<T>> oriented;
		oriented.push_back(data);

		switch (symmetry)
		{
		case Symmetry::I:
		case Symmetry::backslash:
			oriented.push_back(data.GetRotated());
			break;
		case Symmetry::T:
		case Symmetry::L:
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			break;
		case Symmetry::P:
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated().GetReflected());
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			oriented.push_back(data = data.GetRotated());
			break;
		default:
			break;
		}

		return oriented;
	}

	//Create a tile with its different orientations, its symmetries and its
	//weight on the distribution of tiles
	Tile(std::vector<Array2D<T>> data, Symmetry symmetry, double weight, std::string name)
		: data(data), symmetry(symmetry), weight(weight), tileName(name) {}

	//Create a tile with its base orientation, its symmetries and its
	//weight on the distribution of tiles.
	//The other orientation are generated with its first one.
	Tile(Array2D<T> data, Symmetry symmetry, double weight, std::string name)
		: data(GenerateOriented(data, symmetry)), symmetry(symmetry),
		weight(weight), tileName(name) {}
};
