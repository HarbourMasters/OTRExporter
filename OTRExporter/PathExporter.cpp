#include "PathExporter.h"
#include "../ZAPD/ZFile.h"

void OTRExporter_Path::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZPath* path = (ZPath*)res;

	WriteHeader(res, outPath, writer, OtrLib::ResourceType::OTRPath);
	
	writer->Write((uint32_t)path->pathways[0].points.size());

	for (int i = 0; i < path->pathways[0].points.size(); i++)
	{
		writer->Write(path->pathways[0].points[i].scalars[0].scalarData.s16);
		writer->Write(path->pathways[0].points[i].scalars[1].scalarData.s16);
		writer->Write(path->pathways[0].points[i].scalars[2].scalarData.s16);
	}
}