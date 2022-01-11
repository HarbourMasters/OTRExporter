#pragma once

#include "ZResource.h"
#include "ZVtx.h"
#include "Exporter.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_Vtx : public OTRExporter
{
public:
	void SaveArr(const std::vector<ZResource*>&, BinaryWriter* writer);
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};