#include "DisplayListExporter.h"
#include "Main.h"
#include "../ZAPD/ZFile.h"
#include <Utils/MemoryStream.cpp>
#include "Lib/StrHash64.h"
#include "spdlog/spdlog.h"
//#include "Lib/Fast3D/U64/PR/gbi.h"

#define GFX_SIZE 8

void OTRExporter_DisplayList::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZDisplayList* dList = (ZDisplayList*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRDisplayList);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	while (writer->GetBaseAddress() % 8 != 0)
		writer->Write((uint8_t)0xFF);

	for (auto data : dList->instructions)
	{
		uint32_t word0 = 0;
		uint32_t word1 = 0;
		uint8_t opcode = (uint8_t)(data >> 56);
		F3DZEXOpcode opF3D = (F3DZEXOpcode)opcode;



		if (opF3D == F3DZEXOpcode::G_DL)
			opcode = (uint8_t)F3DZEXOpcode_OTR::G_DL_OTR;

		word0 += (opcode << 24);

		switch (opF3D)
		{
		case F3DZEXOpcode::G_NOOP:
		case F3DZEXOpcode::G_RDPPIPESYNC:
		default:
		{
			//writer->Seek(-1, SeekOffsetType::Current);
			//writer->Write(data);
		}
		break;
		break;
		case F3DZEXOpcode::G_DL:
		{
			// TODO:
			Declaration* dListDecl = dList->parent->GetDeclaration(GETSEGOFFSET(data));
			int bp = 0;

			writer->Write(word0);
			writer->Write(word1);
			
			if (dListDecl != nullptr)
			{
				std::string vName = StringHelper::Sprintf("%s\\%s", (StringHelper::Split(res->parent->GetOutName(), "_")[0] + "_scene").c_str(), dListDecl->varName.c_str());

				word0 = CRC64(vName.c_str()) >> 32;
				word1 = CRC64(vName.c_str()) & 0xFFFFFFFF;
			}
			else
			{
				word0 = 0;
				word1 = 0;
				spdlog::error(StringHelper::Sprintf("dListDecl == nullptr! Addr = %08X", GETSEGOFFSET(data)));
			}

			for (int i = 0; i < dList->otherDLists.size(); i++)
			{
				Declaration* dListDecl2 = dList->parent->GetDeclaration(GETSEGOFFSET(dList->otherDLists[i]->GetRawDataIndex()));

				if (dListDecl2 != nullptr)
				{
					MemoryStream* dlStream = new MemoryStream();
					BinaryWriter dlWriter = BinaryWriter(dlStream);

					Save(dList->otherDLists[i], outPath, &dlWriter);

					std::string fName = StringHelper::Sprintf("%s\\%s", (StringHelper::Split(res->parent->GetOutName(), "_")[0] + "_scene").c_str(), dListDecl2->varName.c_str());

					if (otrArchive->HasFile(fName))
						otrArchive->RemoveFile(fName);

					otrArchive->AddFile(fName, (uintptr_t)dlStream->ToVector().data(), dlWriter.GetBaseAddress());
				}
				else
				{
					spdlog::error(StringHelper::Sprintf("dListDecl2 == nullptr! Addr = %08X", GETSEGOFFSET(data)));

				}
			}

			//writer->Write(CRC64(dListDecl->varName.c_str()));
		}
		break;
		case F3DZEXOpcode::G_SETTIMG:
		{
			int bp = 0;

			uint32_t seg = data & 0xFFFFFFFF;
			int32_t texAddress = Seg2Filespace(data, dList->parent->baseAddress);
			Declaration* texDecl = dList->parent->GetDeclaration(texAddress);

			writer->Write(word0);
			writer->Write(word1);

			if (texDecl != nullptr)
			{
				word0 = CRC64(texDecl->varName.c_str()) >> 32;
				word1 = CRC64(texDecl->varName.c_str()) & 0xFFFFFFFF;

				//writer->Write(CRC64(texDecl->varName.c_str()));
			}
			else
			{
				//writer->Write((uint64_t)0);
				word0 = 0;
				word1 = 0;
				spdlog::error("texDecl == nullptr!");
			}

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
					//writer->Write(nn);
					word1 = nn;

					writer->Write(word0);
					writer->Write(word1);

					//while (writer->GetBaseAddress() % GFX_SIZE != 0)
						//writer->Write((uint8_t)0x00);

					//writer->Write(CRC64(vtxDecl->varName.c_str()));

					word0 = CRC64(vtxDecl->varName.c_str()) >> 32;
					word1 = CRC64(vtxDecl->varName.c_str()) & 0xFFFFFFFF;

					// Write vertices to file
					MemoryStream* vtxStream = new MemoryStream();
					BinaryWriter vtxWriter = BinaryWriter(vtxStream);

					int sz = dList->vertices[GETSEGOFFSET(data)].size();

					if (sz > 0)
					{
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

						std::string fName = StringHelper::Sprintf("%s\\%s", (StringHelper::Split(res->parent->GetOutName(), "_")[0] + "_scene").c_str(), vtxDecl->varName.c_str());

						if (otrArchive->HasFile(fName))
							otrArchive->RemoveFile(fName);
						
						otrArchive->AddFile(fName, (uintptr_t)vtxStream->ToVector().data(), vtxWriter.GetBaseAddress());
					}
				}
				else
				{
					spdlog::error("vtxDecl == nullptr!");
				}
			}
			else
			{
				writer->Write(word0);
				writer->Write(word1);
				word0 = 0;
				word1 = 0;

				spdlog::error("dList->vertices.size() <= 0!");
			}
		}
		break;
		}
	
		writer->Write(word0);
		writer->Write(word1);
		
		//for (int i = 0; i < GFX_SIZE - 1; i++)
		//while (writer->GetBaseAddress() % GFX_SIZE != 0)
			//writer->Write((uint8_t)0x00);
	}
}