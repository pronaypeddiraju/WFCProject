#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/FastWFCEntry.hpp"
#include "Game/FastWFCOverlappingModel.hpp"
#include "Game/WFCColor.hpp"
#include "Game/WFCImage.hpp"

//Global settings for WFC
WFCSettings_T gWFCSettings;

//Read the overlapping WFC problem from the XML node
void ReadOverlappingInstance(tinyxml2::XMLElement* node)
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
	float startTime = (float)GetCurrentTimeSeconds();
	DebuggerPrintf("\n Start Time: %f", startTime);
	
	const std::string image_path = gWFCSettings.imageReadPath + name + ".png";
	std::optional<Array2D<Color>> imageColorArray = ReadImage(image_path);
	
	if (!imageColorArray.has_value())
	{
		throw "Error while loading " + image_path;
	}

	OverlappingWFCOptions options = { periodicInput, periodicOutput, height, width, symmetry, ground, N };

	for (unsigned i = 0; i < numOutputImages; i++) 
	{
		for (unsigned test = 0; test < 10; test++) 
		{
			int seed = g_RNG->GetRandomIntInRange(0, INT_MAX);
			OverlappingWFC overlappingWFC(*imageColorArray, options, seed);
			std::optional<Array2D<Color>> success = overlappingWFC.Run();
			
			if (success.has_value()) 
			{
				WriteImageAsPNG(gWFCSettings.imageOutPath + name + std::to_string(i) + ".png", *success);
				DebuggerPrintf("\n Finished solving problem %s", name.c_str());
				break;
			}
			else 
			{
				DebuggerPrintf("\n Failed to solve problem %s", name.c_str());
			}
		}
	}

	float endTime = (float)GetCurrentTimeSeconds();
	DebuggerPrintf("\n End Time: %f", endTime);
	float timeTaken = endTime - startTime;
	DebuggerPrintf("\n Time take for problem: %f", timeTaken);
}

void SetTimeStampedOutPath()
{
	gWFCSettings.imageOutPath += GetDateTime();
	gWFCSettings.imageOutPath += "/";
}

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
	tinyxml2::XMLElement* root = meshDoc.RootElement();
	tinyxml2::XMLElement* node = root->FirstChildElement("overlapping");

	while (node != nullptr)
	{
		ReadOverlappingInstance(node);

		node = node->NextSiblingElement("overlapping");
	}
}

void WFCEntryPoint()
{
	ReadConfigFile(gWFCSettings.configReadPath + gWFCSettings.configFileName);
}