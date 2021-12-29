#include "OTRMtxExporter.h"
#include <OTRResource.h>

void OTRExporter_Mtx::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZMtx* mtx = (ZMtx*)res;
	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRMatrix);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); //id

	for (size_t i = 0; i < 4; i++)
		for (size_t j = 0; j < 4; j++)
			//TODO possibly utilize the array class better
			writer->Write(mtx->mtx[i][j]);
}