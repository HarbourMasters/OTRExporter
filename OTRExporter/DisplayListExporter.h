#pragma once

#include "ZResource.h"
#include "ZTexture.h"
#include "ZDisplayList.h"
#include <Utils/BinaryWriter.h>

enum class F3DZEXOpcode_OTR
{
	G_SETTIMGOTR = 0x20,
};

class OTRExporter_DisplayList : public ZResourceExporter
{
public:
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};