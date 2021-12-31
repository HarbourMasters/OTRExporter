#pragma once
#include "ZResource.h"
#include "ZArray.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_Array : public ZResourceExporter
{
public:
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;

};