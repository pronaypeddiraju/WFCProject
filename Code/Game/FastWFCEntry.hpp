#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <optional>
#include "Engine/Core/XMLUtils/XMLUtils.hpp"
#include "Engine/Commons/EngineCommon.hpp"

const std::string configReadPath = "samples.xml";
const std::string imageOutPath = "WFCResults/";
const uint defaultWidth = 48;
const uint defaultHeight = 48;
const uint defaultNumOutputImages = 2;