#include "ArrayExporter.h"
#include "VtxExporter.h"
void OTRExporter_Array::Save(ZResource* res, const fs::path& outPath, BinaryWriter* writer)
{
	ZArray* arr = (ZArray*)res;

	WriteHeader(res, outPath, writer, Ship::ResourceType::Array);

	writer->Write((uint32_t)arr->resList[0]->GetResourceType());
	writer->Write((uint32_t)arr->arrayCnt);

	for (int i = 0; i < arr->arrayCnt; i++)
	{
		if (arr->resList[i]->GetResourceType() == ZResourceType::Vertex)
		{
			ZVtx* vtx = (ZVtx*)arr->resList[i];
			writer->Write(vtx->x);
			writer->Write(vtx->y);
			writer->Write(vtx->z);
			writer->Write(vtx->flag);
			writer->Write(vtx->s);
			writer->Write(vtx->t);
			writer->Write(vtx->r);
			writer->Write(vtx->g);
			writer->Write(vtx->b);
			writer->Write(vtx->a);
		}
		else
		{
			ZScalar* scal = (ZScalar*)arr->resList[i];

			writer->Write((uint32_t)scal->scalarType);

			switch (scal->scalarType)
			{
			case ZScalarType::ZSCALAR_U8:
				writer->Write(scal->scalarData.u8);
				break;
			case ZScalarType::ZSCALAR_S8:
				writer->Write(scal->scalarData.s8);
				break;
			case ZScalarType::ZSCALAR_U16:
				writer->Write(scal->scalarData.u16);
				break;
			case ZScalarType::ZSCALAR_S16:
				writer->Write(scal->scalarData.s16);
				break;
			case ZScalarType::ZSCALAR_S32:
				writer->Write(scal->scalarData.s32);
				break;
			case ZScalarType::ZSCALAR_U32:
				writer->Write(scal->scalarData.u32);
				break;
			case ZScalarType::ZSCALAR_S64:
				writer->Write(scal->scalarData.s64);
				break;
			case ZScalarType::ZSCALAR_U64:
				writer->Write(scal->scalarData.u64);
				break;
				// OTRTODO: ADD OTHER TYPES
			}
		}
	}


	////There has to be a better way™
	//if (arr->resList[0]->GetResourceType() == ZResourceType::Vertex) 
	//{
	//	OTRExporter_Vtx exporter;
	//	exporter.SaveArr(arr->resList,writer);

	//	//exporter.count = arr->arrayCnt;
	//	//exporter.Save(res, outPath, writer);
	//}
	//else if (arr->resList[0]->GetResourceType() == ZResourceType::Vector)
	//{
	//	WriteHeader(res, outPath, writer, Ship::ResourceType::Vtx);
	//}
}