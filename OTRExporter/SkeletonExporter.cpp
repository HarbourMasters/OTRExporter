#include "SkeletonExporter.h"
#include <Resource.h>
#include <Globals.h>

void OTRExporter_Skeleton::Save(ZResource* res, const fs::path& outPath, BinaryWriter* writer)
{
	ZSkeleton* skel = (ZSkeleton*)res;

	WriteHeader(res, outPath, writer, Ship::ResourceType::Skeleton);

	writer->Write((uint8_t)skel->type);
	writer->Write((uint8_t)skel->limbType);

	writer->Write((uint32_t)skel->limbCount);
	writer->Write((uint32_t)skel->dListCount);

	writer->Write((uint8_t)skel->limbsTable.limbType);
	writer->Write((uint32_t)skel->limbsTable.count);

	for (size_t i = 0; i < skel->limbsTable.count; i++)
	{
		Declaration* skelDecl = skel->parent->GetDeclarationRanged(GETSEGOFFSET(skel->limbsTable.limbsAddresses[i]));

		std::string name;
		bool foundDecl = Globals::Instance->GetSegmentedPtrName(skel->limbsTable.limbsAddresses[i], skel->parent, "", name);
		if (foundDecl)
		{
			if (name.at(0) == '&')
				name.erase(0, 1);

			std::string fName = StringHelper::Sprintf("%s\\%s", skel->parent->GetOutName().c_str(), name.c_str());
			writer->Write(fName);
		}
		else
		{
			writer->Write("");
		}
	}
}
