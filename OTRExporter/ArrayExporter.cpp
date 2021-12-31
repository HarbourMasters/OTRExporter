#include "ArrayExporter.h"
#include "VtxExporter.h"
void OTRExporter_Array::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZArray* arr = (ZArray*)res;
	//There has to be a better way
	if (arr->resList[0]->GetResourceType() == ZResourceType::Vertex) {
		OTRExporter_Vtx exporter;
		exporter.SaveArr(arr->resList,writer);

		//exporter.count = arr->arrayCnt;
		//exporter.Save(res, outPath, writer);
	}

}