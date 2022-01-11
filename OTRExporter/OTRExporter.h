#pragma once
#include "ZResource.h"
#include "ZArray.h"
#include "OTRExporter.h"
#include <Utils/BinaryWriter.h>
#include <Resource.h>

class OTRExporter : public ZResourceExporter
{
protected:
	void WriteHeader(ZResource* res, fs::path outPath, BinaryWriter* writer, Ship::ResourceType resType);
};