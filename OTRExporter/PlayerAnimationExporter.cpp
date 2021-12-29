#include "PlayerAnimationExporter.h"
#include <OTRResource.h>

void OTRExporter_PlayerAnimationExporter::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZPlayerAnimationData* anim = (ZPlayerAnimationData*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRPlayerAnimation);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	auto start = std::chrono::steady_clock::now();
	
	writer->Write((uint32_t)anim->limbRotData.size());

	for (int i = 0; i < anim->limbRotData.size(); i++)
		writer->Write(anim->limbRotData[i]);

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	printf("Exported Player Anim %s in %llims\n", anim->GetName().c_str(), diff);
}
