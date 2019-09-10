#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <optional>
#include "Engine/Core/XMLUtils/XMLUtils.hpp"
#include "Engine/Commons/EngineCommon.hpp"

const std::string configReadPath = "Data/Gameplay/";
const std::string imageReadPath = "Data/Images/WFCRead/";
const std::string configFileName = "samples.xml";
const std::string imageOutPath = "Data/WFCResults/";
const uint defaultWidth = 48;
const uint defaultHeight = 48;
const uint defaultNumOutputImages = 2;

void WFCEntryPoint();