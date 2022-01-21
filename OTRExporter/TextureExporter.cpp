#include "TextureExporter.h"
#include "../ZAPD/ZFile.h"

void OTRExporter_Texture::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZTexture* tex = (ZTexture*)res;
	
	WriteHeader(tex, outPath, writer, Ship::ResourceType::Texture);

	auto start = std::chrono::steady_clock::now();

	//printf("Exporting Texture %s\n", tex->GetName().c_str());
	
	writer->Write((uint32_t)tex->GetTextureType());
	writer->Write((uint32_t)tex->GetWidth());
	writer->Write((uint32_t)tex->GetHeight());

	writer->Write((uint32_t)tex->GetRawDataSize());

	auto data = tex->parent->GetRawData();

	for (size_t i = tex->GetRawDataIndex(); i < tex->GetRawDataIndex() + tex->GetRawDataSize(); i++)
		writer->Write(data[i]);

	auto end = std::chrono::steady_clock::now();
	size_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//printf("Exported Texture %s in %zums\n", tex->GetName().c_str(), diff);

	//if (diff > 2)
		//printf("Export took %lms\n", diff);

	//otrArchive->AddFile(file->GetName(), (uintptr_t)strem->ToVector().data(), strem->GetLength());
}