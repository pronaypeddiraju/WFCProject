#include "Game/FastWFCEntry.hpp"
#include "Game/WFCImage.hpp"
#include "Game/WFCColor.hpp"
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/FastWFCOverlappingModel.hpp"

//Read the overlapping WFC problem from the XML node
void ReadOverlappingInstance(tinyxml2::XMLElement* node)
{
	std::string name = ParseXmlAttribute(*node, "name", "");
	uint N = ParseXmlAttribute(*node, "N", 3);
	bool periodicOutput = ParseXmlAttribute(*node, "periodic", false);
	bool periodicInput = ParseXmlAttribute(*node, "periodicInput", true);
	bool ground = ParseXmlAttribute(*node, "ground", false);

	uint symmetry = ParseXmlAttribute(*node, "symmetry", 8);
	uint numOutputImages = ParseXmlAttribute(*node, "screenshots", defaultNumOutputImages);
	
	uint width = ParseXmlAttribute(*node, "width", defaultWidth);
	uint height = ParseXmlAttribute(*node, "height", defaultHeight);

	DebuggerPrintf("\n Started WFC for Overlapping problem %s", name.c_str());
	
	const std::string image_path = imageReadPath + name + ".png";
	std::optional<Array2D<Color>> imageColorArray = read_image(image_path);
	
	if (!imageColorArray.has_value())
	{
		throw "Error while loading " + image_path;
	}

	OverlappingWFCOptions options = { periodicInput, periodicOutput, height, width, symmetry, ground, N };

	for (unsigned i = 0; i < numOutputImages; i++) 
	{
		for (unsigned test = 0; test < 10; test++) 
		{
			int seed = g_RNG->GetCurrentSeed();
			OverlappingWFC<Color> overlappingWFC(*imageColorArray, options, seed);
			std::optional<Array2D<Color>> success = overlappingWFC.run();
			
			if (success.has_value()) 
			{
				write_image_png(imageOutPath + name + std::to_string(i) + ".png", *success);
				DebuggerPrintf("\n Finised solving problem %s", name.c_str());
				break;
			}
			else 
			{
				DebuggerPrintf("\n Failed to solve problem %s", name.c_str());
			}
		}
	}
}

//Read the config file for the WFC problems
void ReadConfigFile(const std::string &config_path) noexcept 
{
	//Open the xml file and parse it
	tinyxml2::XMLDocument meshDoc;
	meshDoc.LoadFile(config_path.c_str());

	if (meshDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{

		ERROR_AND_DIE(">> Error loading Mesh XML file ");
		return;
	}
	
	//We loaded the file successfully
	tinyxml2::XMLElement* root = meshDoc.RootElement();
	tinyxml2::XMLElement* node = root->FirstChildElement("overlapping");

	while (node != nullptr)
	{
		ReadOverlappingInstance(node);

		node = node->NextSiblingElement("overlapping");
	}

	/*
	for (xml_node<> *node = root_node->first_node("simpletiled"); node;
		node = node->next_sibling("simpletiled")) {
		read_simpletiled_instance(node, dir_path);
	}
	*/
}

void WFCEntryPoint()
{
	ReadConfigFile(configReadPath + configFileName);
}