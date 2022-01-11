#include "SkeletonLimbExporter.h"
#include "DisplayListExporter.h"
#include <Resource.h>

void OTRExporter_SkeletonLimb::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZLimb* limb = (ZLimb*)res;

	WriteHeader(res, outPath, writer, Ship::ResourceType::OTRSkeletonLimb);

	writer->Write((uint8_t)limb->type);
	writer->Write((uint8_t)limb->skinSegmentType);

	writer->Write(limb->segmentStruct.unk_0);
	writer->Write((uint32_t)limb->segmentStruct.unk_4_arr.size());
	
	for (auto item : limb->segmentStruct.unk_4_arr)
	{
		writer->Write(item.unk_4);

		writer->Write((uint32_t)item.unk_8_arr.size());

		for (auto item2 : item.unk_8_arr)
		{
			writer->Write(item2.unk_0);
			writer->Write(item2.unk_2);
			writer->Write(item2.unk_4);
			writer->Write(item2.unk_6);
			writer->Write(item2.unk_7);
			writer->Write(item2.unk_8);
			writer->Write(item2.unk_9);
		}

		writer->Write((uint32_t)item.unk_C_arr.size());

		for (auto item2 : item.unk_C_arr)
		{
			writer->Write(item2.unk_0);
			writer->Write(item2.x);
			writer->Write(item2.y);
			writer->Write(item2.z);
			writer->Write(item2.unk_8);
		}
	}

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