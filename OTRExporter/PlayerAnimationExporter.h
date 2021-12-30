#pragma once

#include "ZResource.h"
#include "ZTexture.h"
#include "ZPlayerAnimationData.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_PlayerAnimationExporter : public ZResourceExporter
{
public:
	virtual void Save(ZResource* res, const fs::path outPath, BinaryWriter* writer) override;
};