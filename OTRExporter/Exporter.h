#pragma once
#include "ZResource.h"
#include "ZArray.h"
#include "stdint.h"
#include <Utils/BinaryWriter.h>
#include <libultraship/bridge.h>
#include "VersionInfo.h"
#include "../../mm/2s2h/resource/type/2shResourceType.h"

class OTRExporter : public ZResourceExporter
{
protected:
	static void WriteHeader(ZResource* res, const fs::path& outPath, BinaryWriter* writer, uint32_t resType, int32_t resVersion = 0);
};
