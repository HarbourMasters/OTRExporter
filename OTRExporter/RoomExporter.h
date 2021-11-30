#pragma once

#include "ZResource.h"
#include "ZRoom/ZRoom.h"

class PolygonDlist;

class OTRExporter_Room : public ZResourceExporter
{
public:
	void WritePolyDList(BinaryWriter* writer, PolygonDlist* dlist);
	virtual void Save(ZResource* res, fs::path outPath, BinaryWriter* writer) override;
};