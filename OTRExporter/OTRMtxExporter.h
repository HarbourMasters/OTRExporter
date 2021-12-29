#pragma once
#include "ZResource.h"
#include "ZMtx.h"

class OTRExporter_Mtx : public ZResourceExporter
{
public:
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};