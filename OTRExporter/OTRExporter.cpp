#include "OTRExporter.h"

void OTRExporter::WriteHeader(ZResource* res, fs::path outPath, BinaryWriter* writer, OtrLib::ResourceType resType)
{
	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)resType);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id
}
