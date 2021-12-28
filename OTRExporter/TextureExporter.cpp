#include "TextureExporter.h"
#include "../ZAPD/ZFile.h"

void OTRExporter_Texture::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZTexture* tex = (ZTexture*)res;

	auto data = tex->parent->GetRawData();

	for (size_t i = tex->GetRawDataIndex(); i < tex->GetRawDataIndex() + tex->GetRawDataSize(); i++)
		writer->Write(data[i]);

	//otrArchive->AddFile(file->GetName(), (uintptr_t)strem->ToVector().data(), strem->GetLength());
}