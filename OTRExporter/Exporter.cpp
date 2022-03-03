#include "Exporter.h"
#include "VersionInfo.h"

void OTRExporter::WriteHeader(ZResource* res, const fs::path& outPath, BinaryWriter* writer, Ship::ResourceType resType)
{
	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)resType);
	writer->Write((uint32_t)MAJOR_VERSION);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id
	writer->Write((uint32_t)resourceVersions[resType]); // id
}
