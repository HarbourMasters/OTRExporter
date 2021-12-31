#pragma once

#include "ZResource.h"
#include "ZVtx.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_Vtx : public ZResourceExporter
{
public:
	void SaveArr(const std::vector<ZResource*>&, BinaryWriter* writer);
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};