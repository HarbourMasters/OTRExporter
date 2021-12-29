#include "BackgroundExporter.h"
#include "../ZAPD/ZFile.h"

void OTRExporter_Background::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZBackground* bg = (ZBackground*)res;
	
	auto start = std::chrono::steady_clock::now();

	//printf("Exporting BG %s\n", tex->GetName().c_str());

	auto data = bg->parent->GetRawData();

	for (int i = bg->GetRawDataIndex(); i < bg->GetRawDataIndex() + bg->GetRawDataSize(); i++)
		writer->Write(data[i]);

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	printf("Exported BG %s in %lims\n", bg->GetName().c_str(), diff);
}