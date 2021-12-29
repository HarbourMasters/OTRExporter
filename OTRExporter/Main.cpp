#include <OTRArchive.h>
//#include <Factories/OTRArchiveFactory.h>
#include "BackgroundExporter.h"
#include "TextureExporter.h"
#include "RoomExporter.h"
#include "CollisionExporter.h"
#include "DisplayListExporter.h"
#include "PlayerAnimationExporter.h"
#include "OTRSkeletonExporter.h"
#include "OTRSkeletonLimbExporter.h"
#include <Globals.h>
#include <Utils/File.h>
#include <Utils/Directory.h>
#include <Utils/MemoryStream.h>
#include <Utils/BinaryWriter.h>

std::shared_ptr<OtrLib::OTRArchive> otrArchive;
BinaryWriter* fileWriter;
std::chrono::steady_clock::time_point fileStart, resStart;

enum class ExporterFileMode
{
	ModeExample1 = (int)ZFileMode::Custom + 1,
	ModeExample2 = (int)ZFileMode::Custom + 2,
	ModeExample3 = (int)ZFileMode::Custom + 3,
};

static void ExporterParseFileMode(std::string buildMode, ZFileMode& fileMode)
{
	if (buildMode == "me1")
		fileMode = (ZFileMode)ExporterFileMode::ModeExample1;
	else if (buildMode == "me2")
		fileMode = (ZFileMode)ExporterFileMode::ModeExample2;
	else if (buildMode == "me3")
		fileMode = (ZFileMode)ExporterFileMode::ModeExample3;
}

static void ExporterParseArgs(int argc, char* argv[], int& i)
{
	std::string arg = argv[i];

	if (arg == "--do-x")
	{

	}
	else if (arg == "--do-y")
	{

	}
}

static bool ExporterProcessFileMode(ZFileMode fileMode)
{
	// Do whatever work is associated with these custom file modes...
	// Return true to indicate one of our own file modes is being processed
	if (fileMode == (ZFileMode)ExporterFileMode::ModeExample1)
		return true;
	else if (fileMode == (ZFileMode)ExporterFileMode::ModeExample2)
		return true;
	else if (fileMode == (ZFileMode)ExporterFileMode::ModeExample3)
		return true;

	return false;
}

static void ExporterFileBegin(ZFile* file)
{
	printf("ExporterFileBegin() called on ZFile %s.\n", file->GetName().c_str());

	fileStart = std::chrono::steady_clock::now();

	MemoryStream* stream = new MemoryStream();
	fileWriter = new BinaryWriter(stream);
}

static void ExporterFileEnd(ZFile* file)
{
	printf("ExporterFileEnd() called on ZFile %s.\n", file->GetName().c_str());

	auto fileEnd = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(fileEnd - fileStart).count();

	printf("File Export Ended %s in %lims\n", file->GetName().c_str(), diff);

	//MemoryStream* strem = (MemoryStream*)fileWriter->GetStream().get();
	//otrArchive->AddFile(file->GetName(), (uintptr_t)strem->ToVector().data(), strem->GetLength());

	//File::WriteAllBytes(StringHelper::Sprintf("conv/%s"));
}

static void ExporterResourceEnd(ZResource* res, BinaryWriter& writer)
{
	MemoryStream* strem = (MemoryStream*)writer.GetStream().get();

	auto start = std::chrono::steady_clock::now();

	if (res->GetName() != "")
	{
		std::string oName = res->parent->GetOutName();
		std::string rName = res->GetName();

		//if (res->GetResourceType() == ZResourceType::Room || res->GetResourceType() == ZResourceType::Scene)
		if (StringHelper::Contains(oName, "_scene") || StringHelper::Contains(oName, "_room"))
		{
			//oName = StringHelper::Split(oName, "_")[0];
			oName = StringHelper::Split(oName, "_")[0] + "_scene";
			//rName = StringHelper::Split(rName, "_")[0];
		}

		std::string fName = StringHelper::Sprintf("%s\\%s", oName.c_str(), rName.c_str());

#ifdef _DEBUG
		if (otrArchive->HasFile(fName))
			otrArchive->RemoveFile(fName);
#endif

		//if (!otrArchive->HasFile(fName))
			otrArchive->AddFile(fName, (uintptr_t)strem->ToVector().data(), writer.GetBaseAddress());
	}

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//if (diff > 10)
		printf("Exported Resource End %s in %lims\n", res->GetName().c_str(), diff);
}

static void ExporterXMLBegin()
{
	printf("ExporterXMLBegin() called.\n");

	if (File::Exists("oot.otr"))
		otrArchive = std::shared_ptr<OtrLib::OTRArchive>(new OtrLib::OTRArchive("oot.otr"));
	else
		otrArchive = OtrLib::OTRArchive::CreateArchive("oot.otr");
}

static void ExporterXMLEnd()
{
	printf("ExporterXMLEnd() called.\n");

	//otrArchive;
}

static void ImportExporters()
{
	// In this example we set up a new exporter called "EXAMPLE".
	// By running ZAPD with the argument -se EXAMPLE, we tell it that we want to use this exporter for our resources.
	ExporterSet* exporterSet = new ExporterSet();
	exporterSet->processFileModeFunc = ExporterProcessFileMode;
	//exporterSet->parseFileModeFunc = ExporterParseFileMode;
	exporterSet->parseArgsFunc = ExporterParseArgs;
	exporterSet->beginFileFunc = ExporterFileBegin;
	exporterSet->endFileFunc = ExporterFileEnd;
	exporterSet->beginXMLFunc = ExporterXMLBegin;
	exporterSet->endXMLFunc = ExporterXMLEnd;
	exporterSet->resSaveFunc = ExporterResourceEnd;
	exporterSet->exporters[ZResourceType::Background] = new OTRExporter_Background();
	exporterSet->exporters[ZResourceType::Texture] = new OTRExporter_Texture();
	exporterSet->exporters[ZResourceType::Room] = new OTRExporter_Room();
	exporterSet->exporters[ZResourceType::Scene] = new OTRExporter_Room();
	exporterSet->exporters[ZResourceType::CollisionHeader] = new OTRExporter_Collision();
	exporterSet->exporters[ZResourceType::DisplayList] = new OTRExporter_DisplayList();
	exporterSet->exporters[ZResourceType::PlayerAnimationData] = new OTRExporter_PlayerAnimationExporter();
	exporterSet->exporters[ZResourceType::Skeleton] = new OTRExporter_Skeleton();
	exporterSet->exporters[ZResourceType::Limb] = new OTRExporter_SkeletonLimb();

	Globals::AddExporter("OTR", exporterSet);
}

// When ZAPD starts up, it will automatically call the below function, which in turn sets up our exporters.
REGISTER_EXPORTER(ImportExporters);
