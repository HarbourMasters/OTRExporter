#pragma once

#include "ZResource.h"
#include "ZBackground.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_Background : public ZResourceExporter
{
public:
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};