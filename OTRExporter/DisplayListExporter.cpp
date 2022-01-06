#include "DisplayListExporter.h"
#include "Main.h"
#include "../ZAPD/ZFile.h"
#include <Utils/MemoryStream.h>
#include <Utils/BitConverter.h>
#include "Lib/StrHash64.h"
#include "spdlog/spdlog.h"
#include "PR/ultra64/gbi.h"
#include <Globals.h>
#include <iostream>
#include <string>
//#include "gbi.h"
//#include "Lib/Fast3D/U64/PR/gbi.h"

#define GFX_SIZE 8

#define gsDPSetCombineLERP2(a0, b0, c0, d0, Aa0, Ab0, Ac0, Ad0,      \
        a1, b1, c1, d1, Aa1, Ab1, Ac1, Ad1)         \
{                                   \
    _SHIFTL(G_SETCOMBINE, 24, 8) |                  \
    _SHIFTL(GCCc0w0(a0, c0,         \
               Aa0, Ac0) |          \
           GCCc1w0(a1, c1), 0, 24),     \
    (unsigned int)(GCCc0w1(b0, d0,      \
                   Ab0, Ad0) |      \
               GCCc1w1(b1, Aa1,     \
                   Ac1, d1,     \
                   Ab1, Ad1))       \
}

typedef int32_t Mtx_t[4][4];

typedef union Mtx
{
	//_Alignas(8)
		Mtx_t		m;
	int32_t		l[16];
	struct
	{
		int16_t		i[16];
		uint16_t	f[16];
	};
} Mtx;

void OTRExporter_DisplayList::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZDisplayList* dList = (ZDisplayList*)res;

	printf("Exporting DList %s\n", dList->GetName().c_str());

	WriteHeader(res, outPath, writer, OtrLib::ResourceType::OTRDisplayList);

	while (writer->GetBaseAddress() % 8 != 0)
		writer->Write((uint8_t)0xFF);

	// DEBUG: Write in a marker
	Declaration* dbgDecl = dList->parent->GetDeclaration(dList->GetRawDataIndex());
	std::string dbgName = StringHelper::Sprintf("%s\\%s", GetParentFolderName(res).c_str(), dbgDecl->varName.c_str());
	uint64_t hash = CRC64(dbgName.c_str());
	writer->Write((uint32_t)(G_MARKER << 24));
	writer->Write((uint32_t)0xBEEFBEEF);
	writer->Write((uint32_t)(hash >> 32));
	writer->Write((uint32_t)(hash & 0xFFFFFFFF));

	auto dlStart = std::chrono::steady_clock::now();

	for (auto data : dList->instructions)
	{
		uint32_t word0 = 0;
		uint32_t word1 = 0;
		uint8_t opcode = (uint8_t)(data >> 56);
		F3DZEXOpcode opF3D = (F3DZEXOpcode)opcode;

		if ((int)opF3D == G_DL)
			opcode = (uint8_t)G_DL_OTR;

		if ((int)opF3D == G_VTX)
			opcode = (uint8_t)G_VTX_OTR;

		if ((int)opF3D == G_SETTIMG)
			opcode = (uint8_t)G_SETTIMG_OTR;

		word0 += (opcode << 24);

		switch ((int)opF3D)
		{
		case G_NOOP:
		{
			Gfx value = gsDPNoOp();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_ENDDL:
		{
			Gfx value = gsSPEndDisplayList();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		default:
		{
			printf("Undefined opcode: %02X\n", opcode);
			//word0 = _byteswap_ulong((uint32_t)(data >> 32));
			//word1 = _byteswap_ulong((uint32_t)(data & 0xFFFFFFFF));
		}
		break;
		case G_GEOMETRYMODE:
		{
			int32_t cccccc = (data & 0x00FFFFFF00000000) >> 32;
			int32_t ssssssss = (data & 0xFFFFFFFF);

			Gfx value = gsSPGeometryMode(~cccccc, ssssssss);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_RDPPIPESYNC:
		{
			Gfx value = gsDPPipeSync();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_RDPLOADSYNC:
		{
			Gfx value = gsDPLoadSync();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_RDPTILESYNC:
		{
			Gfx value = gsDPTileSync();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_RDPFULLSYNC:
		{
			Gfx value = gsDPFullSync();
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_RDPSETOTHERMODE:
		{
			int32_t hhhhhh = (data & 0x00FFFFFF00000000) >> 32;
			int32_t llllllll = (data & 0x00000000FFFFFFFF);

			Gfx value = gsDPSetOtherMode(hhhhhh, llllllll);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_POPMTX:
		{
			Gfx value = gsSPPopMatrix(data);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETENVCOLOR:
		{
			uint8_t r = (uint8_t)((data & 0xFF000000) >> 24);
			uint8_t g = (uint8_t)((data & 0x00FF0000) >> 16);
			uint8_t b = (uint8_t)((data & 0xFF00FF00) >> 8);
			uint8_t a = (uint8_t)((data & 0x000000FF) >> 0);

			Gfx value = gsDPSetEnvColor(r, g, b, a);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_MTX:
		{
			uint32_t pp = (data & 0x000000FF00000000) >> 32;
			uint32_t mm = (data & 0x00000000FFFFFFFF);

			pp ^= G_MTX_PUSH;

			mm = (mm & 0x0FFFFFFF) + 0xF0000000;

			Gfx value = gsSPMatrix(mm, pp);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_LOADBLOCK:
		{
			int32_t sss = (data & 0x00FFF00000000000) >> 48;
			int32_t ttt = (data & 0x00000FFF00000000) >> 36;
			int32_t i = (data & 0x000000000F000000) >> 24;
			int32_t xxx = (data & 0x0000000000FFF000) >> 12;
			int32_t ddd = (data & 0x0000000000000FFF);

			Gfx value = gsDPLoadBlock(i, sss, ttt, xxx, ddd);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_CULLDL:
		{
			int32_t vvvv = (data & 0xFFFF00000000) >> 32;
			int32_t wwww = (data & 0x0000FFFF);

			Gfx value = gsSPCullDisplayList(vvvv / 2, wwww / 2);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_DL:
		{
			if (!Globals::Instance->HasSegment(GETSEGNUM(data)))
			{
				int32_t pp = (data & 0x00FF000000000000) >> 56;

				Gfx value;

				u32 dListVal = (data & 0x0FFFFFFF) + 0xF0000000;

				if (pp != 0)
					value = gsSPBranchList(dListVal);
				else
					value = gsSPDisplayList(dListVal);

				word0 = value.words.w0;
				word1 = value.words.w1;
			}
			else
			{
				Declaration* dListDecl = dList->parent->GetDeclaration(GETSEGOFFSET(data));
				int bp = 0;

				writer->Write(word0);
				writer->Write(word1);

				if (dListDecl != nullptr)
				{
					std::string vName = StringHelper::Sprintf("%s\\%s", (GetParentFolderName(res).c_str()), dListDecl->varName.c_str());

					uint64_t hash = CRC64(vName.c_str());

					word0 = hash >> 32;
					word1 = hash & 0xFFFFFFFF;
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

						std::string fName = StringHelper::Sprintf("%s\\%s", GetParentFolderName(res).c_str(), dListDecl2->varName.c_str());

#ifdef _DEBUG
						if (otrArchive->HasFile(fName))
							otrArchive->RemoveFile(fName);
#endif

						otrArchive->AddFile(fName, (uintptr_t)dlStream->ToVector().data(), dlWriter.GetBaseAddress());
					}
					else
					{
						spdlog::error(StringHelper::Sprintf("dListDecl2 == nullptr! Addr = %08X", GETSEGOFFSET(data)));
					}
				}
			}
		}
		break;
		case G_TEXTURE:
		{
			int32_t ____ = (data & 0x0000FFFF00000000) >> 32;
			int32_t ssss = (data & 0x00000000FFFF0000) >> 16;
			int32_t tttt = (data & 0x000000000000FFFF);
			int32_t lll = (____ & 0x3800) >> 11;
			int32_t ddd = (____ & 0x700) >> 8;
			int32_t nnnnnnn = (____ & 0xFE) >> 1;

			Gfx value = gsSPTexture(ssss, tttt, lll, ddd, nnnnnnn);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_TRI1:
		{
			int32_t aa = ((data & 0x00FF000000000000ULL) >> 48) / 2;
			int32_t bb = ((data & 0x0000FF0000000000ULL) >> 40) / 2;
			int32_t cc = ((data & 0x000000FF00000000ULL) >> 32) / 2;

			Gfx test = gsSP1Triangle(aa, bb, cc, 0);
			word0 = test.words.w0;
			word1 = test.words.w1;
		}
		break;
		case G_TRI2:
		{
			int32_t aa = ((data & 0x00FF000000000000ULL) >> 48) / 2;
			int32_t bb = ((data & 0x0000FF0000000000ULL) >> 40) / 2;
			int32_t cc = ((data & 0x000000FF00000000ULL) >> 32) / 2;
			int32_t dd = ((data & 0x00000000FF0000ULL) >> 16) / 2;
			int32_t ee = ((data & 0x0000000000FF00ULL) >> 8) / 2;
			int32_t ff = ((data & 0x000000000000FFULL) >> 0) / 2;

			Gfx test = gsSP2Triangles(aa, bb, cc, 0, dd, ee, ff, 0);
			word0 = test.words.w0;
			word1 = test.words.w1;
		}
		break;
		/*case G_QUAD:
		{
			gsSP1Quadrangle()
		}
			break;*/
		case G_SETPRIMCOLOR:
		{
			int32_t mm = (data & 0x0000FF0000000000) >> 40;
			int32_t ff = (data & 0x000000FF00000000) >> 32;
			int32_t rr = (data & 0x00000000FF000000) >> 24;
			int32_t gg = (data & 0x0000000000FF0000) >> 16;
			int32_t bb = (data & 0x000000000000FF00) >> 8;
			int32_t aa = (data & 0x00000000000000FF) >> 0;

			Gfx value = gsDPSetPrimColor(mm, ff, rr, gg, bb, aa);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETOTHERMODE_L:
		{
			int32_t ss = (data & 0x0000FF0000000000) >> 40;
			int32_t len = ((data & 0x000000FF00000000) >> 32) + 1;
			int32_t sft = 32 - (len)-ss;
			int32_t dd = (data & 0xFFFFFFFF);

			// TODO: Output the correct render modes in data

			Gfx value = gsSPSetOtherMode(0xE2, sft, len, dd);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETOTHERMODE_H:
		{
			int32_t ss = (data & 0x0000FF0000000000) >> 40;
			int32_t nn = (data & 0x000000FF00000000) >> 32;
			int32_t dd = (data & 0xFFFFFFFF);

			int32_t sft = 32 - (nn + 1) - ss;

			Gfx value;

			if (sft == 14)  // G_MDSFT_TEXTLUT
			{
				const char* types[] = { "G_TT_NONE", "G_TT_NONE", "G_TT_RGBA16", "G_TT_IA16" };
				value = gsDPSetTextureLUT(dd >> 14);
			}
			else
			{
				value = gsSPSetOtherMode(0xE3, sft, nn + 1, dd);
			}

			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETTILE:
		{
			int32_t fff = (data & 0b0000000011100000000000000000000000000000000000000000000000000000) >> 53;
			int32_t ii = (data & 0b0000000000011000000000000000000000000000000000000000000000000000) >> 51;
			int32_t nnnnnnnnn =
				(data & 0b0000000000000011111111100000000000000000000000000000000000000000) >> 41;
			int32_t mmmmmmmmm =
				(data & 0b0000000000000000000000011111111100000000000000000000000000000000) >> 32;
			int32_t ttt = (data & 0b0000000000000000000000000000000000000111000000000000000000000000) >> 24;
			int32_t pppp =
				(data & 0b0000000000000000000000000000000000000000111100000000000000000000) >> 20;
			int32_t cc = (data & 0b0000000000000000000000000000000000000000000011000000000000000000) >> 18;
			int32_t aaaa =
				(data & 0b0000000000000000000000000000000000000000000000111100000000000000) >> 14;
			int32_t ssss =
				(data & 0b0000000000000000000000000000000000000000000000000011110000000000) >> 10;
			int32_t dd = (data & 0b0000000000000000000000000000000000000000000000000000001100000000) >> 8;
			int32_t bbbb = (data & 0b0000000000000000000000000000000000000000000000000000000011110000) >> 4;
			int32_t uuuu = (data & 0b0000000000000000000000000000000000000000000000000000000000001111);

			Gfx value = gsDPSetTile(fff, ii, nnnnnnnnn, mmmmmmmmm, ttt, pppp, cc, aaaa, ssss, dd, bbbb, uuuu);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETCOMBINE:
		{
			int32_t a0 = (data & 0b000000011110000000000000000000000000000000000000000000000000000) >> 52;
			int32_t c0 = (data & 0b000000000001111100000000000000000000000000000000000000000000000) >> 47;
			int32_t aa0 = (data & 0b00000000000000011100000000000000000000000000000000000000000000) >> 44;
			int32_t ac0 = (data & 0b00000000000000000011100000000000000000000000000000000000000000) >> 41;
			int32_t a1 = (data & 0b000000000000000000000011110000000000000000000000000000000000000) >> 37;
			int32_t c1 = (data & 0b000000000000000000000000001111100000000000000000000000000000000) >> 32;
			int32_t b0 = (data & 0b000000000000000000000000000000011110000000000000000000000000000) >> 28;
			int32_t b1 = (data & 0b000000000000000000000000000000000001111000000000000000000000000) >> 24;
			int32_t aa1 = (data & 0b00000000000000000000000000000000000000111000000000000000000000) >> 21;
			int32_t ac1 = (data & 0b00000000000000000000000000000000000000000111000000000000000000) >> 18;
			int32_t d0 = (data & 0b000000000000000000000000000000000000000000000111000000000000000) >> 15;
			int32_t ab0 = (data & 0b00000000000000000000000000000000000000000000000111000000000000) >> 12;
			int32_t ad0 = (data & 0b00000000000000000000000000000000000000000000000000111000000000) >> 9;
			int32_t d1 = (data & 0b000000000000000000000000000000000000000000000000000000111000000) >> 6;
			int32_t ab1 = (data & 0b00000000000000000000000000000000000000000000000000000000111000) >> 3;
			int32_t ad1 = (data & 0b00000000000000000000000000000000000000000000000000000000000111) >> 0;

			Gfx value = gsDPSetCombineLERP2(a0, b0, c0, d0, aa0, ab0, ac0, ad0, a1, b1, c1, d1, aa1, ab1, ac1, ad1);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETTILESIZE:
		{
			int32_t sss = (data & 0x00FFF00000000000) >> 44;
			int32_t ttt = (data & 0x00000FFF00000000) >> 32;
			int32_t uuu = (data & 0x0000000000FFF000) >> 12;
			int32_t vvv = (data & 0x0000000000000FFF);
			int32_t i = (data & 0x000000000F000000) >> 24;

			Gfx value = gsDPSetTileSize(i, sss, ttt, uuu, vvv);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_LOADTLUT:
		{
			int32_t t = (data & 0x0000000007000000) >> 24;
			int32_t ccc = (data & 0x00000000003FF000) >> 14;

			Gfx value = gsDPLoadTLUTCmd(t, ccc);
			word0 = value.words.w0;
			word1 = value.words.w1;
		}
		break;
		case G_SETTIMG:
		{
			uint32_t seg = data & 0xFFFFFFFF;
			int32_t texAddress = Seg2Filespace(data, dList->parent->baseAddress);

			if (StringHelper::Contains(res->GetName(), "gLinkChildHatNearDL"))
			{
				int bp = 0;
			}

			if (!Globals::Instance->HasSegment(GETSEGNUM(seg)))
			{
				int32_t __ = (data & 0x00FF000000000000) >> 48;
				int32_t www = (data & 0x00000FFF00000000) >> 32;

				uint32_t fmt = (__ & 0xE0) >> 5;
				uint32_t siz = (__ & 0x18) >> 3;

				Gfx value = gsDPSetTextureImage(fmt, siz, www - 1, (seg & 0x0FFFFFFF) + 0xF0000000);
				word0 = value.words.w0;
				word1 = value.words.w1;

				writer->Write(word0);
				writer->Write(word1);
			}
			else
			{
				std::string texName = "";
				bool foundDecl = Globals::Instance->GetSegmentedPtrName(seg, dList->parent, "", texName);

				int32_t __ = (data & 0x00FF000000000000) >> 48;
				int32_t www = (data & 0x00000FFF00000000) >> 32;

				uint32_t fmt = (__ & 0xE0) >> 5;
				uint32_t siz = (__ & 0x18) >> 3;

				Gfx value = gsDPSetTextureImage(fmt, siz, www - 1, __);
				word0 = value.words.w0 & 0x00FFFFFF;
				word0 += (G_SETTIMG_OTR << 24);
				word1 = value.words.w1;

				writer->Write(word0);
				writer->Write(word1);

				if (foundDecl)
				{
					ZFile* assocFile = Globals::Instance->GetSegment(GETSEGNUM(seg));
					std::string assocFileName = assocFile->GetName();
					std::string fName = "";
					
					if (GETSEGNUM(seg) == SEGMENT_SCENE || GETSEGNUM(seg) == SEGMENT_ROOM)
						fName = StringHelper::Sprintf("%s\\%s", GetParentFolderName(res).c_str(), texName.c_str());
					else
						fName = StringHelper::Sprintf("%s\\%s", assocFileName.c_str(), texName.c_str());
					
					uint64_t hash = CRC64(fName.c_str());

					word0 = hash >> 32;
					word1 = hash & 0xFFFFFFFF;
				}
				else
				{
					word0 = 0;
					word1 = 0;
					spdlog::error("texDecl == nullptr! PTR = 0x%08X", texAddress);
				}
			}
		}
		break;
		case G_VTX:
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
				auto segOffset = GETSEGOFFSET(data);
				uint32_t seg = data & 0xFFFFFFFF;
				Declaration* vtxDecl = dList->parent->GetDeclarationRanged(segOffset);
				//std::string vtxName = "";
				//bool foundDecl = Globals::Instance->GetSegmentedPtrName(seg, dList->parent, "", vtxName);

				int32_t aa = (data & 0x000000FF00000000ULL) >> 32;
				int32_t nn = (data & 0x000FF00000000000ULL) >> 44;

				if (vtxDecl != nullptr)
				{
					uint32_t diff = segOffset - vtxDecl->address;

					Gfx value = gsSPVertex(diff, nn, ((aa >> 1) - nn));

					word0 = value.words.w0;
					word0 &= 0x00FFFFFF;
					word0 += (G_VTX_OTR << 24);
					word1 = value.words.w1;

					writer->Write(word0);
					writer->Write(word1);

					std::string fName = OTRExporter_DisplayList::GetPathToRes(res, vtxDecl->varName);

					printf("Exporting VTX Data %s\n", fName.c_str());

					uint64_t hash = CRC64(fName.c_str());

					word0 = hash >> 32;
					word1 = hash & 0xFFFFFFFF;

					//if (!otrArchive->HasFile(fName))
					{
						// Write vertices to file
						MemoryStream* vtxStream = new MemoryStream();
						BinaryWriter vtxWriter = BinaryWriter(vtxStream);

						size_t sz = dList->vertices[vtxDecl->address].size();

						if (sz > 0)
						{
							auto start = std::chrono::steady_clock::now();


							// God dammit this is so dumb
							auto split = StringHelper::Split(vtxDecl->text, "\n");

							for (int i = 0; i < split.size(); i++)
							{
								std::string line = split[i];

								if (StringHelper::Contains(line, "VTX("))
								{
									auto split2 = StringHelper::Split(StringHelper::Split(StringHelper::Split(line, "VTX(")[1], ")")[0], ",");

									vtxWriter.Write((int16_t)std::stoi(split2[0], nullptr, 10)); // v.x
									vtxWriter.Write((int16_t)std::stoi(split2[1], nullptr, 10)); // v.y
									vtxWriter.Write((int16_t)std::stoi(split2[2], nullptr, 10)); // v.z

									vtxWriter.Write((int16_t)0);								 // v.flag
									
									vtxWriter.Write((int16_t)std::stoi(split2[3], nullptr, 10)); // v.s
									vtxWriter.Write((int16_t)std::stoi(split2[4], nullptr, 10)); // v.t
									
									vtxWriter.Write((uint8_t)std::stoi(split2[5], nullptr, 10)); // v.r
									vtxWriter.Write((uint8_t)std::stoi(split2[6], nullptr, 10)); // v.g
									vtxWriter.Write((uint8_t)std::stoi(split2[7], nullptr, 10)); // v.b
									vtxWriter.Write((uint8_t)std::stoi(split2[8], nullptr, 10)); // v.a

									int bp = 0;
								}
							}

							/*for (size_t i = 0; i < sz; i++)
							{
								auto v = dList->vertices[vtxDecl->address][i];

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
							}*/

							//#ifdef _DEBUG
														//if (otrArchive->HasFile(fName))
															//otrArchive->RemoveFile(fName);
							//#endif

							otrArchive->AddFile(fName, (uintptr_t)vtxStream->ToVector().data(), vtxWriter.GetBaseAddress());

							auto end = std::chrono::steady_clock::now();
							auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

							printf("Exported VTX Array %s in %lims\n", fName.c_str(), diff);
						}
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
	}

	auto dlEnd = std::chrono::steady_clock::now();
	auto dlDiff = std::chrono::duration_cast<std::chrono::milliseconds>(dlEnd - dlStart).count();

	printf("Display List Gen in %lims\n", dlDiff);
}

std::string OTRExporter_DisplayList::GetPathToRes(ZResource* res, std::string varName)
{
	std::string fName = StringHelper::Sprintf("%s\\%s", GetParentFolderName(res).c_str(), varName.c_str());
	return fName;
}

std::string OTRExporter_DisplayList::GetParentFolderName(ZResource* res)
{
	//if (StringHelper::Contains(res->parent->GetOutName(), "_scene") || StringHelper::Contains(res->parent->GetOutName(), "_room"))
		//return (StringHelper::Split(res->parent->GetOutName(), "_")[0] + "_scene");
	//else
		//return res->parent->GetOutName();

	std::string oName = res->parent->GetOutName();

	if (StringHelper::Contains(oName, "_scene"))
	{
		auto split = StringHelper::Split(oName, "_");
		oName = "";
		for (int i = 0; i < split.size() - 1; i++)
			oName += split[i] + "_";

		oName += "scene";

		//oName = StringHelper::Split(oName, "_")[0] + "_scene";
	}
	else if (StringHelper::Contains(oName, "_room"))
	{
		oName = StringHelper::Split(oName, "_room")[0] + "_scene";
	}

	return oName;
}
