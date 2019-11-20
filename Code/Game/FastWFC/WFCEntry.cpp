#include "Game/FastWFC/WFCEntry.hpp"
//------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Commons/LogSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/FastWFC/WFCMarkovModel.hpp"
#include "Game/FastWFC/WFCOverlappingModel.hpp"
#include "Game/FastWFC/WFCTilingModel.hpp"
#include "Game/FastWFC/WFCColor.hpp"
#include "Game/FastWFC/WFCImage.hpp"

//------------------------------------------------------------------------------------------------------------------------------
#include <fstream>
#include <unordered_set>

//Global settings for WFC
WFCSettings_T gWFCSettings;

bool gStoreAllKernels = true;

//------------------------------------------------------------------------------------------------------------------------------
//Parse Symmetry name and turn it into a Symmetry Enum value
Symmetry ToSymmetry(const std::string &symmetryName) 
{
	if (symmetryName == "X") 
	{
		return Symmetry::X;
	}
	if (symmetryName == "T") 
	{
		return Symmetry::T;
	}
	if (symmetryName == "I") 
	{
		return Symmetry::I;
	}
	if (symmetryName == "L") 
	{
		return Symmetry::L;
	}
	if (symmetryName == "\\") 
	{
		return Symmetry::backslash;
	}
	if (symmetryName == "P") 
	{
		return Symmetry::P;
	}
	throw symmetryName + "is an invalid Symmetry";
}

//------------------------------------------------------------------------------------------------------------------------------
//Read the names of the tiles in the subset in Tiling WFC problem
std::optional<std::unordered_set<std::string>> ReadSubsetNames(XMLElement* root, const std::string &subset) 
{
	std::unordered_set<std::string> subsetNames;

	//tinyxml2::XMLDocument* currentDoc = root->GetDocument();
	XMLElement* subsetsNode = root->FirstChildElement("subsets");
	
	if (!subsetsNode) 
	{
		return std::nullopt;
	}
	
	XMLElement* subset_node = subsetsNode->FirstChildElement("subset");

	while (subsetsNode && ParseXmlAttribute(*subset_node, "name") != subset)
	{
		subset_node = subset_node->NextSiblingElement("subset");
	}

	if (subset_node == nullptr)
	{
		return std::nullopt;
	}
	
	for (XMLElement* node = subset_node->FirstChildElement("tile"); node; node = node->NextSiblingElement("tile")) 
	{
		subsetNames.insert(ParseXmlAttribute(*node, "name"));
	}

	return subsetNames;
}

//------------------------------------------------------------------------------------------------------------------------------
//Read all the tiles for a Tiling problem
std::unordered_map<std::string, Tile<Color>> ReadTiles(XMLElement* root, const std::string &currentDir, const std::string &subset, uint size)
{
	std::optional<std::unordered_set<std::string>> subsetNames = ReadSubsetNames(root, subset);

	std::unordered_map<std::string, Tile<Color>> tiles;

	XMLElement* tilesNode = root->FirstChildElement("tiles");

	for (XMLElement *node = tilesNode->FirstChildElement("tile"); node;	node = node->NextSiblingElement("tile")) 
	{
		std::string name = ParseXmlAttribute(*node, "name");
		if (subsetNames != std::nullopt && subsetNames->find(name) == subsetNames->end())
		{
			continue;
		}

		Symmetry symmetry = ToSymmetry(ParseXmlAttribute(*node, "symmetry", "X"));
		double weight = (double)ParseXmlAttribute(*node, "weight", 1.0f);
		
		const std::string imagePath = currentDir + "/" + name + ".png";
		std::optional<Array2D<Color>> image = ReadImage(imagePath);

		//the image read return nullopt
		if (image == std::nullopt) 
		{
			std::vector<Array2D<Color>> images;
			for (unsigned i = 0; i < NumPossibleOrientations(symmetry); i++)
			{
				const std::string subImagePath = currentDir + "/" + name + " " + std::to_string(i) + ".png";
				std::optional<Array2D<Color>> subImage = ReadImage(subImagePath);
				
				if (subImage == std::nullopt)
				{
					throw "Error while loading " + subImagePath;
				}
				if ((subImage->m_width != size) || (subImage->m_height != size))
				{
					throw "Image " + subImagePath + " has wrong size";
				}
				images.push_back(*subImage);
			}
			Tile<Color> tile = { images, symmetry, weight, name };
			tiles.insert({ name, tile });
		}
		else 
		{
			if ((image->m_width != size) || (image->m_height != size)) 
			{
				throw "Image " + imagePath + " has wrong size";
			}

			Tile<Color> tile(*image, symmetry, weight, name);
			tiles.insert({ name, tile });
		}
	}

	return tiles;
}

//------------------------------------------------------------------------------------------------------------------------------
//Read the neighbors constraints for a tiling problem
//Implementation: a value {tile1, orientation1, tile2, orientation2} means tile1 with orientation1 can
// be placed at the right hand side of tile2 with orientation2
//------------------------------------------------------------------------------------------------------------------------------
std::vector<std::tuple<std::string, uint, std::string, uint>> ReadNeighbors(XMLElement* root)
{
	std::vector<std::tuple<std::string, uint, std::string, uint>> neighbors;

	XMLElement* neighborNode = root->FirstChildElement("neighbors");

	for (XMLElement* node = neighborNode->FirstChildElement("neighbor"); node; node = node->NextSiblingElement("neighbor"))
	{
		std::string left = ParseXmlAttribute(*node, "left", "");

		std::string::size_type left_delimiter = left.find(" ");
		std::string left_tile = left.substr(0, left_delimiter);
		
		uint left_orientation = 0U;

		if (left_delimiter != std::string::npos) 
		{
			left_orientation = std::stoi(left.substr(left_delimiter, std::string::npos));
		}

		std::string right = ParseXmlAttribute(*node, "right", "");
		
		std::string::size_type right_delimiter = right.find(" ");
		std::string right_tile = right.substr(0, right_delimiter);
		
		uint right_orientation = 0;
		
		if (right_delimiter != std::string::npos) 
		{
			right_orientation = std::stoi(right.substr(right_delimiter, std::string::npos));
		}
		neighbors.push_back({ left_tile, left_orientation, right_tile, right_orientation });
	}
	return neighbors;
}

//------------------------------------------------------------------------------------------------------------------------------
//Read all the inputs for a Markov problem
std::vector<Array2D<Color>> ReadInputs(XMLElement* root, const std::string &currentDir)
{
	std::vector<Array2D<Color>> inputs;
	XMLElement* inputsNode = root->FirstChildElement("inputs");

	for (XMLElement *node = inputsNode->FirstChildElement("input"); node; node = node->NextSiblingElement("input"))
	{
		std::string name = ParseXmlAttribute(*node, "name");

		const std::string imagePath = currentDir + "/" + name + ".png";
		std::optional<Array2D<Color>> image = ReadImage(imagePath);

		//the image read return nullopt
		if (image == std::nullopt)
		{
			DebuggerPrintf("Error reading inputs for the MarkovWFC problem");
			ERROR_AND_DIE("Could not open input file for Markov WFC problem");
		}
		else
		{
			inputs.push_back(image.value());
		}
	}

	return inputs;
}

//------------------------------------------------------------------------------------------------------------------------------
//Read Markov WFC Problem
void ReadMarkovInstance(tinyxml2::XMLElement* node, int problemIndex, const std::string &currentDir)
{
	std::string name = ParseXmlAttribute(*node, "name", "");
	std::string subset = ParseXmlAttribute(*node, "subset", "tiles");
	bool periodicOutput = ParseXmlAttribute(*node, "periodic", false);
	uint width = ParseXmlAttribute(*node, "width", gWFCSettings.defaultWidth);
	uint height = ParseXmlAttribute(*node, "height", gWFCSettings.defaultHeight);

	DebuggerPrintf("Started Markov Problem %s :  Subset: %s ", name.c_str(), subset.c_str());
	DebuggerPrintf("\n\n Started WFC for Markov problem: %s Subset: %s", name.c_str(), subset.c_str());
	g_LogSystem->Logf("WFC System", "\n\n Started WFC for Markov problem: %s Subset: %s", name.c_str(), subset.c_str());

	float startTime = (float)GetCurrentTimeSeconds();
	float endTime;

	DebuggerPrintf("\n Start Time: %f", startTime);
	g_LogSystem->Logf("WFC System", "\n Start Time: %f", startTime);

	//Update the path for current problem
	std::string readDir = currentDir + "/" + name + "/data.xml";

	//Open the xml file and parse it
	tinyxml2::XMLDocument dataDocument;
	dataDocument.LoadFile(readDir.c_str());

	if (dataDocument.ErrorID() != tinyxml2::XML_SUCCESS)
	{
		std::string error = Stringf(">> Error loading Data document for Markov Tiled problem number: %d", problemIndex);
		ERROR_AND_DIE(error.c_str());
		return;
	}

	//We loaded the file successfully
	XMLElement* root = dataDocument.RootElement();
	uint size = ParseXmlAttribute(*root, "size", 0);

	std::string assertString = Stringf("The size attriutes for the Markov Set was 0. Problem number: %d", problemIndex);
	ASSERT_OR_DIE(size != 0U, assertString.c_str());

	std::unordered_map<std::string, Tile<Color>> tilesMap = ReadTiles(root, currentDir + "/" + name, subset, size);
	std::unordered_map<std::string, uint> tilesID;
	std::vector<Tile<Color>> tiles;

	unsigned id = 0;
	for (std::pair<std::string, Tile<Color>> tile : tilesMap)
	{
		tilesID.insert({ tile.first, id });
		tiles.push_back(tile.second);
		id++;
	}

	MarkovWFCOptions options;
	options.m_width = width;
	options.m_height = height;
	options.m_periodicOutput = periodicOutput;
	options.m_tileSize = size;

	std::vector<Array2D<Color>> inputs = ReadInputs(root, currentDir + "/" + name);

	//Write all the patterns to a patterns folder
	std::string outFolderPath = gWFCSettings.imageOutPath + name;
	outFolderPath += "/";
	g_windowContext->CheckCreateDirectory(outFolderPath.c_str());

	std::string outFolderKernelsPath = outFolderPath;
	outFolderKernelsPath += "/Kernels/";
	if (gStoreAllKernels)
	{
		g_windowContext->CheckCreateDirectory(outFolderKernelsPath.c_str());
	}

	//Let's account for different problems with the same name
	outFolderPath += "Problem_" + std::to_string(problemIndex) + "_";
	outFolderKernelsPath += "/Problem_" + std::to_string(problemIndex) + "_";

	for (uint test = 0; test < 10; test++)
	{
		int seed = g_RNG->GetRandomIntInRange(0, INT_MAX);

		MarkovWFC<Color> wfc(tiles, inputs, height, width, options, seed);

		std::optional<Array2D<Color>> success = wfc.Run();
		if (success.has_value())
		{
			WriteImageAsPNG(outFolderPath + name + "_" + subset + "_" + std::to_string(test) + ".png", *success);

			DebuggerPrintf("\n Finished solving Markov problem: %s subset: %s", name.c_str(), subset.c_str());
			g_LogSystem->Logf("WFC System", "\n Finished solving Markov problem: %s subset: %s", name.c_str(), subset.c_str());

			endTime = (float)GetCurrentTimeSeconds();
			g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);

			break;
		}
		else
		{
			DebuggerPrintf("\n Failed to solve Markov problem: %s subset: %s", name.c_str(), subset.c_str());
			g_LogSystem->Logf("WFC System", "\n Failed to solve Markov problem: %s subset: %s", name.c_str(), subset.c_str());

			endTime = (float)GetCurrentTimeSeconds();
			g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);
		}
	}

	float timeTaken = endTime - startTime;
	DebuggerPrintf("\n Time take for Markov problem: %f", timeTaken);
	g_LogSystem->Logf("WFC System", "\n Time take for Markov problem: %f", timeTaken);
}


//------------------------------------------------------------------------------------------------------------------------------
//Read Tiling WFC Problem
void ReadSimpleTiledInstance(tinyxml2::XMLElement* node, int problemIndex, const std::string &currentDir)
{
	std::string name = ParseXmlAttribute(*node, "name", "");
	std::string subset = ParseXmlAttribute(*node, "subset", "tiles");
	bool periodicOutput = ParseXmlAttribute(*node, "periodic", false);
	uint width = ParseXmlAttribute(*node, "width", gWFCSettings.defaultWidth);
	uint height = ParseXmlAttribute(*node, "height", gWFCSettings.defaultHeight);

	DebuggerPrintf("Started SimpleTiled Problem %s :  Subset: %s ", name.c_str(), subset.c_str());

	DebuggerPrintf("\n\n Started WFC for Tiled problem: %s Subset: %s", name.c_str(), subset.c_str());
	g_LogSystem->Logf("WFC System", "\n\n Started WFC for Tiling problem: %s Subset: %s", name.c_str(), subset.c_str());

	float startTime = (float)GetCurrentTimeSeconds();
	float endTime;

	DebuggerPrintf("\n Start Time: %f", startTime);
	g_LogSystem->Logf("WFC System", "\n Start Time: %f", startTime);

	//Update the path for current problem
	std::string readDir = currentDir + "/" + name + "/data.xml";

	//Open the xml file and parse it
	tinyxml2::XMLDocument dataDocument;
	dataDocument.LoadFile(readDir.c_str());

	if (dataDocument.ErrorID() != tinyxml2::XML_SUCCESS)
	{
		std::string error = Stringf(">> Error loading Data document for Simple Tiled problem number: %d", problemIndex);
		ERROR_AND_DIE(error.c_str());
		return;
	}

	//We loaded the file successfully
	XMLElement* root = dataDocument.RootElement();
	uint size = ParseXmlAttribute(*root, "size", 0);
	
	std::string assertString = Stringf("The size attributes for the SimpleTiled Set was 0. Problem number: %d", problemIndex);
	ASSERT_OR_DIE(size != 0U, assertString.c_str());

	std::unordered_map<std::string, Tile<Color>> tilesMap =	ReadTiles(root, currentDir + "/" + name, subset, size);
	std::unordered_map<std::string, uint> tilesID;
	std::vector<Tile<Color>> tiles;
	
	unsigned id = 0;
	for (std::pair<std::string, Tile<Color>> tile : tilesMap) 
	{
		tilesID.insert({ tile.first, id });
		tiles.push_back(tile.second);
		id++;
	}

	std::vector<std::tuple<std::string, uint, std::string, uint>> neighbors = ReadNeighbors(root);

	std::vector<std::tuple<uint, uint, uint, uint>> neighborsIDs;
	
	for (auto neighbor : neighbors) 
	{
		const std::string &neighbor1 = std::get<0>(neighbor);
		const int &orientation1 = std::get<1>(neighbor);
		const std::string &neighbor2 = std::get<2>(neighbor);
		const int &orientation2 = std::get<3>(neighbor);
		if (tilesID.find(neighbor1) == tilesID.end()) 
		{
			continue;
		}
		if (tilesID.find(neighbor2) == tilesID.end()) 
		{
			continue;
		}
		
		neighborsIDs.push_back(std::make_tuple(tilesID[neighbor1], orientation1,	tilesID[neighbor2], orientation2));
	}

	//Write all the patterns to a patterns folder
	std::string outFolderPath = gWFCSettings.imageOutPath + name;
	outFolderPath += "/";
	g_windowContext->CheckCreateDirectory(outFolderPath.c_str());

	std::string outFolderKernelsPath = outFolderPath;
	outFolderKernelsPath += "/Kernels/";
	if (gStoreAllKernels)
	{
		g_windowContext->CheckCreateDirectory(outFolderKernelsPath.c_str());
	}

	//Let's account for different problems with the same name
	outFolderPath += "Problem_" + std::to_string(problemIndex) + "_";
	outFolderKernelsPath += "/Problem_" + std::to_string(problemIndex) + "_";

	for (uint test = 0; test < 10; test++) 
	{
		int seed = g_RNG->GetRandomIntInRange(0, INT_MAX);

		TilingWFC<Color> wfc(tiles, neighborsIDs, height, width, { periodicOutput }, seed);

		std::optional<Array2D<Color>> success = wfc.Run();
		if (success.has_value()) 
		{
			WriteImageAsPNG(outFolderPath + name + "_" + subset + "_" + std::to_string(test) + ".png", *success);

			DebuggerPrintf("\n Finished solving tiling problem: %s subset: %s", name.c_str(), subset.c_str());
			g_LogSystem->Logf("WFC System", "\n Finished solving tiling problem: %s subset: %s", name.c_str(), subset.c_str());


			endTime = (float)GetCurrentTimeSeconds();
			g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);
			break;
		}
		else
		{
			DebuggerPrintf("\n Failed to solve tiling problem: %s subset: %s", name.c_str(), subset.c_str());
			g_LogSystem->Logf("WFC System", "\n Failed to solve tiling problem: %s subset: %s", name.c_str(), subset.c_str());

			endTime = (float)GetCurrentTimeSeconds();
			g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);
		}
	}

	float timeTaken = endTime - startTime;
	DebuggerPrintf("\n Time take for problem: %f", timeTaken);
	g_LogSystem->Logf("WFC System", "\n Time take for Tiling problem: %f", timeTaken);
}

//------------------------------------------------------------------------------------------------------------------------------
//Read the overlapping WFC problem from the XML node
void ReadOverlappingInstance(tinyxml2::XMLElement* node, int problemIndex)
{
	std::string name = ParseXmlAttribute(*node, "name", "");
	uint N = ParseXmlAttribute(*node, "N", 3);
	bool periodicOutput = ParseXmlAttribute(*node, "periodic", false);
	bool periodicInput = ParseXmlAttribute(*node, "periodicInput", true);
	std::string groundString = ParseXmlAttribute(*node, "ground", "0");
	//If ground is anything but 0 set to true
	bool ground = (stoi(groundString) != 0);

	uint symmetry = ParseXmlAttribute(*node, "symmetry", 8);
	uint numOutputImages = ParseXmlAttribute(*node, "screenshots", gWFCSettings.defaultNumOutputImages);

	uint width = ParseXmlAttribute(*node, "width", gWFCSettings.defaultWidth);
	uint height = ParseXmlAttribute(*node, "height", gWFCSettings.defaultHeight);

	DebuggerPrintf("\n\n Started WFC for Overlapping problem %s", name.c_str());
	g_LogSystem->Logf("WFC System", "\n\n Started WFC for Overlapping problem %s", name.c_str());

	float startTime = (float)GetCurrentTimeSeconds();
	float endTime;

	DebuggerPrintf("\n Start Time: %f", startTime);
	g_LogSystem->Logf("WFC System", "\n Start Time: %f", startTime);

	const std::string image_path = gWFCSettings.imageReadPath + name + ".png";
	std::optional<Array2D<Color>> imageColorArray = ReadImage(image_path);

	if (!imageColorArray.has_value())
	{
		throw "Error while loading " + image_path;
	}

	OverlappingWFCOptions options = { periodicInput, periodicOutput, height, width, symmetry, ground, N };

	//Write all the patterns to a patterns folder
	std::string outFolderPath = gWFCSettings.imageOutPath + name;
	outFolderPath += "/";
	g_windowContext->CheckCreateDirectory(outFolderPath.c_str());

	std::string outFolderKernelsPath = outFolderPath;
	outFolderKernelsPath += "/Kernels/";
	if (gStoreAllKernels)
	{
		g_windowContext->CheckCreateDirectory(outFolderKernelsPath.c_str());
	}

	//Let's account for different problems with the same name
	outFolderPath += "/Problem_" + std::to_string(problemIndex) + "_";
	outFolderKernelsPath += "/Problem_" + std::to_string(problemIndex) + "_";

	for (uint i = 0; i < numOutputImages; i++)
	{
		for (uint test = 0; test < 10; test++)
		{
			int seed = g_RNG->GetRandomIntInRange(0, INT_MAX);
			OverlappingWFC overlappingWFC(*imageColorArray, options, seed);
			std::optional<Array2D<Color>> success = overlappingWFC.Run();

			if (success.has_value())
			{
				if (gStoreAllKernels)
				{
					const std::vector<Array2D<Color>>& patterns = overlappingWFC.GetPatterns();

					for (int patternIndex = 0; patternIndex < patterns.size(); patternIndex++)
					{
						WriteImageAsPNG(outFolderKernelsPath + "Run_" + std::to_string(i) + "_Kernel_" + std::to_string(patternIndex) + ".png", patterns[patternIndex]);
					}
				}

				WriteImageAsPNG(outFolderPath + name + "_" + std::to_string(i) + ".png", *success);
				DebuggerPrintf("\n Finished solving problem %s", name.c_str());
				g_LogSystem->Logf("WFC System", "\n Finished solving Overlapping problem %s", name.c_str());

				endTime = (float)GetCurrentTimeSeconds();
				g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);

				break;
			}
			else
			{
				DebuggerPrintf("\n Failed to solve problem %s", name.c_str());
				g_LogSystem->Logf("WFC System", "\n Failed to solve Overlapping problem %s", name.c_str());

				endTime = (float)GetCurrentTimeSeconds();
				g_LogSystem->Logf("WFC System", "\n End Time: %f", endTime);
			}
		}
	}

	float timeTaken = endTime - startTime;
	DebuggerPrintf("\n Time take for problem: %f", timeTaken);
	g_LogSystem->Logf("WFC System", "\n Time take for Overlapping problem: %f", timeTaken);
}

//------------------------------------------------------------------------------------------------------------------------------
void SetTimeStampedOutPath()
{
	gWFCSettings.imageOutPath += GetDateTime();
	gWFCSettings.imageOutPath += "/";
}

//------------------------------------------------------------------------------------------------------------------------------
//Read the config file for the WFC problems
void ReadConfigFile(const std::string &config_path) noexcept
{
	SetTimeStampedOutPath();
	g_windowContext->CheckCreateDirectory(gWFCSettings.imageOutPath.c_str());

	//Open the xml file and parse it
	tinyxml2::XMLDocument meshDoc;
	meshDoc.LoadFile(config_path.c_str());

	if (meshDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{

		ERROR_AND_DIE(">> Error loading Mesh XML file ");
		return;
	}

	//We loaded the file successfully
	//Now let's read all the Overlapping problems
	tinyxml2::XMLElement* root = meshDoc.RootElement();
	tinyxml2::XMLElement* node = root->FirstChildElement("overlapping");

	int problemIndex = 1;

	while (node != nullptr)
	{
		ReadOverlappingInstance(node, problemIndex);
		node = node->NextSiblingElement("overlapping");

		++problemIndex;
	}

	std::string tiledModelDir = GetDirectoryFromFilePath(gWFCSettings.imageReadPath.c_str());

	//Let's read all SimpleTiled problems
	root = meshDoc.RootElement();
	node = root->FirstChildElement("simpletiled");

	while (node != nullptr)
	{
		ReadSimpleTiledInstance(node, problemIndex, tiledModelDir);
		node = node->NextSiblingElement("simpletiled");

		++problemIndex;
	}
	
	//Let's read all the markov problems
	root = meshDoc.RootElement();
	node = root->FirstChildElement("markov");

	while (node != nullptr)
	{
		ReadMarkovInstance(node, problemIndex, tiledModelDir);
		node = node->NextSiblingElement("markov");

		++problemIndex;
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void WFCEntryPoint()
{
	ReadConfigFile(gWFCSettings.configReadPath + gWFCSettings.configFileName);
}