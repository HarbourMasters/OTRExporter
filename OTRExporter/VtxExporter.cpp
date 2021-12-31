#include "VtxExporter.h"
#include "OTRResource.h"


void OTRExporter_Vtx::SaveArr(const std::vector<ZResource*>& vec, BinaryWriter* writer)
{
	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRVtx);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id
	for (auto& res: vec) {
		ZVtx* vtx = (ZVtx*)res;
		writer->Write(vtx->x);
		writer->Write(vtx->y);
		writer->Write(vtx->z);
		writer->Write(vtx->flag);
		writer->Write(vtx->s);
		writer->Write(vtx->t);
		writer->Write(vtx->r);
		writer->Write(vtx->g);
		writer->Write(vtx->b);
		writer->Write(vtx->a);
	}

}

void OTRExporter_Vtx::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZVtx* vtx = (ZVtx*)res;
	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRVtx);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	writer->Write(vtx->x);
	writer->Write(vtx->y);
	writer->Write(vtx->z);
	writer->Write(vtx->flag);
	writer->Write(vtx->s);
	writer->Write(vtx->t);
	writer->Write(vtx->r);
	writer->Write(vtx->g);
	writer->Write(vtx->b);
	writer->Write(vtx->a);



}
