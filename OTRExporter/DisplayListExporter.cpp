#include "DisplayListExporter.h"
#include "Main.h"
#include "../ZAPD/ZFile.h"
#include <Utils/MemoryStream.cpp>
#include "Lib/StrHash64.h"
#include "spdlog/spdlog.h"
//#include "Lib/Fast3D/U64/PR/gbi.h"

#define GFX_SIZE 16

void OTRExporter_DisplayList::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZDisplayList* dList = (ZDisplayList*)res;

	for (auto data : dList->instructions)
	{
		uint8_t opcode = (uint8_t)(data >> 56);
		F3DZEXOpcode opF3D = (F3DZEXOpcode)opcode;

		writer->Write(opcode);

		switch (opF3D)
		{
		case F3DZEXOpcode::G_NOOP:
		case F3DZEXOpcode::G_RDPPIPESYNC:
		default:
		{
			for (int i = 0; i < GFX_SIZE - 1; i++)
				writer->Write((uint8_t)0x00);
		}
		break;
		break;
		case F3DZEXOpcode::G_DL:
		{
			// TODO:

			Declaration* dListDecl = dList->parent->GetDeclaration(GETSEGOFFSET(data));
			int bp = 0;
		}
		break;
		case F3DZEXOpcode::G_SETTIMG:
		{
			int bp = 0;

			uint32_t seg = data & 0xFFFFFFFF;
			int32_t texAddress = Seg2Filespace(data, dList->parent->baseAddress);
			Declaration* texDecl = dList->parent->GetDeclaration(texAddress);

			if (texDecl != nullptr)
				writer->Write(CRC64(texDecl->varName.c_str()));
			else
				spdlog::error("texDecl == nullptr!");

			//MemoryStream* vtxStream = new MemoryStream();
			//BinaryWriter* vtxWriter = new BinaryWriter(vtxStream);

			//otrArchive->AddFile(texDecl->varName, (uintptr_t)vtxStream->ToVector().data(), vtxWriter->GetBaseAddress());
		}
		break;
		case F3DZEXOpcode::G_VTX:
		{
			if (dList->vertices.size() > 0)
			{
				// Connect neighboring vertex arrays
				std::vector<std::pair<uint32_t, std::vector<ZVtx>>> vertsKeys(dList->vertices.begin(),
					dList->vertices.end());

				auto lastItem = vertsKeys[0];

				for (int i = 1; i < vertsKeys.size(); i++)
				{
					auto curItem = vertsKeys[i];

					int32_t sizeDiff = curItem.first - (lastItem.first + (lastItem.second.size() * 16));

					// Make sure there isn't an unaccounted inbetween these two
					if (sizeDiff == 0)
					{
						for (auto v : curItem.second)
						{
							dList->vertices[lastItem.first].push_back(v);
							lastItem.second.push_back(v);
						}

						dList->vertices.erase(curItem.first);
						vertsKeys.erase(vertsKeys.begin() + i);

						i--;
						continue;
					}

					lastItem = curItem;
				}

				// Write CRC64 of vtx file name
				Declaration* vtxDecl = dList->parent->GetDeclarationRanged(GETSEGOFFSET(data));
				int32_t nn = (data & 0x000FF00000000000ULL) >> 44;

				if (vtxDecl != nullptr)
				{
					writer->Write(nn);
					writer->Write(CRC64(vtxDecl->varName.c_str()));

					// Write vertices to file
					MemoryStream* vtxStream = new MemoryStream();
					BinaryWriter vtxWriter = BinaryWriter(vtxStream);

					int sz = dList->vertices[GETSEGOFFSET(data)].size();
					//for (int i = 0; i < nn; i++)
					for (int i = 0; i < sz; i++)
					{
						auto v = dList->vertices[GETSEGOFFSET(data)][i];

						vtxWriter.Write(v.x);
						vtxWriter.Write(v.y);
						vtxWriter.Write(v.z);
						vtxWriter.Write(v.flag);
						vtxWriter.Write(v.s);
						vtxWriter.Write(v.t);
						vtxWriter.Write(v.r);
						vtxWriter.Write(v.g);
						vtxWriter.Write(v.b);
						vtxWriter.Write(v.a);
					}

					std::string fName = StringHelper::Sprintf("%s\\%s", res->parent->GetOutName().c_str(), vtxDecl->varName.c_str());

					if (!otrArchive->HasFile(fName))
						otrArchive->AddFile(fName, (uintptr_t)vtxStream->ToVector().data(), vtxWriter.GetBaseAddress());
				}
				else
				{
					spdlog::error("vtxDecl == nullptr!");
				}
			}
			else
			{
				spdlog::error("dList->vertices.size() <= 0!");
			}
		}
		break;
		}
	}
}