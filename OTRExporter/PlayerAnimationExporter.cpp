#include "PlayerAnimationExporter.h"
#include <OTRResource.h>

void OTRExporter_PlayerAnimationExporter::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZPlayerAnimationData* anim = (ZPlayerAnimationData*)res;

	WriteHeader(res, outPath, writer, OtrLib::ResourceType::OTRPlayerAnimation);

	auto start = std::chrono::steady_clock::now();
	
	writer->Write((uint32_t)anim->limbRotData.size());

	for (int i = 0; i < anim->limbRotData.size(); i++)
		writer->Write(anim->limbRotData[i]);

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	printf("Exported Player Anim %s in %lims\n", anim->GetName().c_str(), diff);
}
