#include "OTRSkeletonLimbExporter.h"
#include "DisplayListExporter.h"
#include <OTRResource.h>

void OTRExporter_SkeletonLimb::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZLimb* limb = (ZLimb*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRSkeletonLimb);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	writer->Write((uint8_t)limb->type);
	writer->Write((uint8_t)limb->skinSegmentType);

	writer->Write(limb->legTransX);
	writer->Write(limb->legTransY);
	writer->Write(limb->legTransZ);
	writer->Write(limb->rotX);
	writer->Write(limb->rotY);
	writer->Write(limb->rotZ);

	// OTRTODO:
	if (limb->childPtr != 0)
	{
		auto childDecl = limb->parent->GetDeclaration(GETSEGOFFSET(limb->childPtr));
		
		if (childDecl != nullptr)
			writer->Write(OTRExporter_DisplayList::GetPathToRes(limb, childDecl->varName));
		else
			writer->Write("");
	}
	else
	{
		writer->Write("");
	}

	if (limb->siblingPtr != 0)
	{
		auto siblingDecl = limb->parent->GetDeclaration(GETSEGOFFSET(limb->siblingPtr));

		if (siblingDecl != nullptr)
			writer->Write(OTRExporter_DisplayList::GetPathToRes(limb, siblingDecl->varName));
		else
			writer->Write("");
	}
	else
	{
		writer->Write("");
	}

	if (limb->dListPtr != 0)
	{
		auto dlDecl = limb->parent->GetDeclaration(GETSEGOFFSET(limb->dListPtr));

		if (dlDecl != nullptr)
			writer->Write(OTRExporter_DisplayList::GetPathToRes(limb, dlDecl->varName));
		else
			writer->Write("");
	}
	else
	{
		writer->Write("");
	}

	if (limb->dList2Ptr != 0)
	{
		auto dlDecl = limb->parent->GetDeclaration(GETSEGOFFSET(limb->dList2Ptr));

		if (dlDecl != nullptr)
			writer->Write(OTRExporter_DisplayList::GetPathToRes(limb, dlDecl->varName));
		else
			writer->Write("");
	}
	else
	{
		writer->Write("");
	}

	writer->Write(limb->transX);
	writer->Write(limb->transY);
	writer->Write(limb->transZ);

	writer->Write(limb->childIndex);
	writer->Write(limb->siblingIndex);
}