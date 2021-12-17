#include "CollisionExporter.h"
#include <OTRResource.h>

void OTRExporter_Collision::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZCollisionHeader* col = (ZCollisionHeader*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRCollisionHeader);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	writer->Write(col->absMinX);
	writer->Write(col->absMinY);
	writer->Write(col->absMinZ);

	writer->Write(col->absMaxX);
	writer->Write(col->absMaxY);
	writer->Write(col->absMaxZ);

	//writer->Write(col->vtxAddress);

	//writer->Write(col->numPolygons);
	//writer->Write(col->polyAddress);
	//writer->Write(col->polyTypeDefAddress);
	//writer->Write(col->camDataAddress);

	//writer->Write(col->numWaterBoxes);
	//writer->Write(col->waterBoxAddress);

	//writer->Write(col->vtxSegmentOffset);
	//writer->Write(col->polySegmentOffset);
	//writer->Write(col->polyTypeDefSegmentOffset);
	//writer->Write(col->camDataSegmentOffset);
	//writer->Write(col->waterBoxSegmentOffset);

	//uint32_t oldOffset = writer->GetBaseAddress();
	//writer->Seek(col->vtxSegmentOffset, SeekOffsetType::Start);

	writer->Write((uint32_t)col->vertices.size());
	
	for (uint16_t i = 0; i < col->vertices.size(); i++)
	{
		writer->Write(col->vertices[i].scalars[0].scalarData.s16);
		writer->Write(col->vertices[i].scalars[1].scalarData.s16);
		writer->Write(col->vertices[i].scalars[2].scalarData.s16);
	}

	//writer->Seek(col->polySegmentOffset, SeekOffsetType::Start);

	writer->Write((uint32_t)col->polygons.size());

	for (uint16_t i = 0; i < col->polygons.size(); i++)
	{
		writer->Write(col->polygons[i].type);
		writer->Write(col->polygons[i].vtxA);
		writer->Write(col->polygons[i].vtxB);
		writer->Write(col->polygons[i].vtxC);
		writer->Write(col->polygons[i].a);
		writer->Write(col->polygons[i].b);
		writer->Write(col->polygons[i].c);
		writer->Write(col->polygons[i].d);
	}

	//writer->Seek(col->polyTypeDefSegmentOffset, SeekOffsetType::Start);
	writer->Write((uint32_t)col->polygonTypes.size());

	for (uint16_t i = 0; i < col->polygonTypes.size(); i++)
		writer->Write(col->polygonTypes[i]);

	//writer->Seek(col->camDataSegmentOffset, SeekOffsetType::Start);
	writer->Write((uint32_t)col->camData->entries.size());

	for (auto entry : col->camData->entries)
	{
		writer->Write(entry->cameraSType);
		writer->Write(entry->numData);
		writer->Write(entry->cameraPosDataSeg);
	}

	//writer->Seek(oldOffset, SeekOffsetType::Start);
}
