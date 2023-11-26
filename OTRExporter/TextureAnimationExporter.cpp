#define NO_GDI
#define WIN32_LEAN_AND_MEAN
#include "ZFile.h"
#include <Globals.h>
#include "TextureAnimationExporter.h"
#include "DisplayListExporter.h"

#undef FindResource

void OTRExporter_TextureAnimation::Save(ZResource* res, const fs::path& outPath, BinaryWriter* writer)
{
	auto* anim = (ZTextureAnimation*)res;
	WriteHeader(res, outPath, writer, LUS::ResourceType::TSH_TexAnim, 0);
	writer->Write((uint32_t)anim->entries.size());

	for (const auto& e : anim->entries) {
		auto* params = (ZTextureAnimationParams*)res->parent->FindResource(Seg2Filespace(e.paramsPtr, res->parent->baseAddress));
		writer->Write(e.segment);
		writer->Write((uint8_t)e.type);
		switch ((TextureAnimationParamsType)e.type) {
			case TextureAnimationParamsType::SingleScroll: 
			case TextureAnimationParamsType::DualScroll: {
				auto* scrollParams = (TextureScrollingParams*)params;
				
				writer->Write(scrollParams->rows[0].xStep);
				writer->Write(scrollParams->rows[0].yStep);
				writer->Write(scrollParams->rows[0].width);
				writer->Write(scrollParams->rows[0].height);
				if (scrollParams->count == 2) {
					writer->Write(scrollParams->rows[1].xStep);
					writer->Write(scrollParams->rows[1].yStep);
					writer->Write(scrollParams->rows[1].width);
					writer->Write(scrollParams->rows[1].height);
				}
				break;
			}

			case TextureAnimationParamsType::ColorChange:
			case TextureAnimationParamsType::ColorChangeLERP:
			case TextureAnimationParamsType::ColorChangeLagrange: {
				auto* colorParams = (TextureColorChangingParams*)params;
				writer->Write(colorParams->animLength);
				writer->Write(colorParams->colorListCount);

				writer->Write((uint32_t)colorParams->frameDataList.size());
				for (const auto f : colorParams->frameDataList) {
					writer->Write(f);
				}
				writer->Write((uint32_t)colorParams->primColorList.size());
				for (const auto prim : colorParams->primColorList) {
					writer->Write(prim.r);
					writer->Write(prim.g);
					writer->Write(prim.b);
					writer->Write(prim.a);
					writer->Write(prim.lodFrac);
				}
				writer->Write((uint16_t)colorParams->envColorList.size());
				for (const auto env : colorParams->envColorList) {
					writer->Write(env.r);
					writer->Write(env.g);
					writer->Write(env.b);
					writer->Write(env.a);
				}
				break;
			}
			case TextureAnimationParamsType::TextureCycle: {
				auto* cycleParams = (TextureCyclingParams*)params;
				
				// cycleLength is the same langth 
				writer->Write(cycleParams->cycleLength);
				//writer->Write(cycleParams->textureList.size());

				for (const auto t : cycleParams->textureList) {
					std::string name;
					//ZTexture* tex = (ZTexture*)res->parent->FindResource(t & 0x00FFFFFF);
					bool found = Globals::Instance->GetSegmentedPtrName(GETSEGOFFSET(t), res->parent, "", name, res->parent->workerID);
					if (found)
					{
						if (name.at(0) == '&')
							name.erase(0, 1);

						writer->Write(OTRExporter_DisplayList::GetPathToRes(res, name));
					}
					else
					{
						writer->Write("");
					}
				}
				//writer->Write(cycleParams->textureIndexList.size());
				for (const auto index : cycleParams->textureIndexList) {
					writer->Write(index);
				}
				break;
			}
			case TextureAnimationParamsType::Empty: {
				writer->Write(SEGMENTED_NULL);
				break;
			}
		}
	}
}