#include "Main.h"
#include "Exporter.h"
#include "BackgroundExporter.h"
#include "TextureExporter.h"
#include "RoomExporter.h"
#include "CollisionExporter.h"
#include "DisplayListExporter.h"
#include "PlayerAnimationExporter.h"
#include "SkeletonExporter.h"
#include "SkeletonLimbExporter.h"
#include "ArrayExporter.h"
#include "VtxExporter.h"
#include "AnimationExporter.h"
#include "CutsceneExporter.h"
#include "PathExporter.h"
#include "TextExporter.h"
#include "TextMMExporter.h"
#include "BlobExporter.h"
#include "MtxExporter.h"
#include "AudioExporter.h"
#include "TextureAnimationExporter.h"
#include "CKeyFrameExporter.h"
#include <Globals.h>
#include <Utils/DiskFile.h>
#include <Utils/Directory.h>
#include <Utils/MemoryStream.h>
#include <Utils/BinaryWriter.h>
#include <Utils/BitConverter.h>
#include <bit>
#include <mutex>
#include <ExporterArchiveO2R.h>

#include "ExporterArchiveOTR.h"
#ifdef GAME_MM
std::string archiveFileName = "mm.o2r";
#elif GAME_OOT
std::string archiveFileName = "oot.o2r";
#endif
std::string customArchiveFileName = "";
std::string customAssetsPath = "";
std::string portVersionString = "0.0.0";

std::shared_ptr<ExporterArchive> archive;
BinaryWriter* fileWriter;
std::chrono::steady_clock::time_point fileStart, resStart;
std::map<std::string, std::vector<char>> files;
std::mutex fileMutex;

void InitVersionInfo();

enum class ExporterFileMode
{
    BuildOTR = (int)ZFileMode::Custom + 1,
};

static void ExporterParseFileMode(const std::string& buildMode, ZFileMode& fileMode)
{
    if (buildMode == "botr")
    {
        fileMode = (ZFileMode)ExporterFileMode::BuildOTR;

        printf("BOTR: Generating OTR Archive...\n");

        archive = std::make_shared<ExporterArchiveO2R>(archiveFileName, true);

        if (DiskFile::Exists(archiveFileName))
            archive->Load(true);
        else
            archive->CreateArchive(40000);

        auto lst = Directory::ListFiles("Extract");

        for (auto item : lst)
        {
            auto fileData = DiskFile::ReadAllBytes(item);
            archive->AddFile(StringHelper::Split(item, "Extract/")[1], fileData.data(), fileData.size());
        }
    }
}

typedef struct Data {
    std::vector<char> fileData;
    std::string filePath;
    size_t size;
} Data;

typedef struct DataU {
    std::vector<uint8_t> fileData;
    std::string filePath;
    size_t size;
} DataU;

static void ExporterProgramEnd()
{
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t endianness = (uint8_t)Endianness::Big;

    std::vector<uint16_t> portVersion = {};
    std::vector<std::string> versionParts = StringHelper::Split(portVersionString, ".");

    // If a major.minor.patch string was not passed in, fallback to 0 0 0
    if (versionParts.size() != 3) {
        portVersion = { 0, 0, 0 };
    } else {
        // Parse version values to number
        for (const auto& val : versionParts) {
            uint16_t num = 0;
            try {
                num = (uint16_t)std::stoi(val, nullptr);
            } catch (std::invalid_argument &e) {
                num = 0;
            } catch (std::out_of_range &e) {
                num = 0;
            }

            portVersion.push_back(num);
        }
    }

    MemoryStream *portVersionStream = new MemoryStream();
    BinaryWriter portVerWriter(portVersionStream);
    portVerWriter.SetEndianness(Endianness::Big);
    portVerWriter.Write(endianness);
    portVerWriter.Write(portVersion[0]); // Major
    portVerWriter.Write(portVersion[1]); // Minor
    portVerWriter.Write(portVersion[2]); // Patch
    portVerWriter.Close();

    if (Globals::Instance->fileMode == ZFileMode::ExtractDirectory)
    {
        std::string romPath = Globals::Instance->baseRomPath.string();
        std::vector<uint8_t> romData = DiskFile::ReadAllBytes(romPath);

        BitConverter::RomToBigEndian(romData.data(), romData.size());

        crc = BitConverter::ToUInt32BE(romData, 0x10);
        printf("Creating version file...\n");

        // Get crc from rom

        MemoryStream *versionStream = new MemoryStream();
        BinaryWriter writer(versionStream);
        writer.SetEndianness(Endianness::Big);
        writer.Write(endianness);
        writer.Write(crc);
        writer.Close();

        printf("Created version file.\n");

        printf("Generating OTR Archive...\n");
        archive = std::make_shared<ExporterArchiveO2R>(archiveFileName, true);
        archive->CreateArchive(40000);

        printf("Adding game version file.\n");
        auto versionStreamBuffer = versionStream->ToVector();
        archive->AddFile("version", (void*)versionStreamBuffer.data(), versionStream->GetLength());

        printf("Adding portVersion file.\n");
        auto portVersionStreamBuffer = portVersionStream->ToVector();
        archive->AddFile("portVersion", (void*)portVersionStreamBuffer.data(), portVersionStream->GetLength());

        // Build a ZRom to access DMA file sizes and MQ status.
        ZRom rom(romPath);
        const auto& romVersion = rom.GetVersion();

        // Export DMA file sizes for the N64 memory model.
        // Parse the DMA table directly from romData using the version's offset and filelist.
        auto fileListFullPath = StringHelper::Sprintf("%s/%s",
            Globals::Instance->fileListPath.string().c_str(), romVersion.listPath.c_str());
        auto fileListText = DiskFile::ReadAllText(fileListFullPath);
        auto fileListLines = StringHelper::Split(fileListText, "\n");

        // Count non-deleted DMA entries first.
        uint32_t dmaEntryCount = 0;
        for (size_t i = 0; i < fileListLines.size(); i++)
        {
            const int romOffset = romVersion.offset + 16 * i;
            const int physStart = BitConverter::ToInt32BE(romData, romOffset + 8);
            if (const int physEnd = BitConverter::ToInt32BE(romData, romOffset + 12); !(physEnd == 0xFFFFFFFF &&
                physStart == 0xFFFFFFFF))
            {
                dmaEntryCount++;
            }
        }

        auto* dmaStream = new MemoryStream();
        BinaryWriter dmaWriter(dmaStream);
        dmaWriter.SetEndianness(Endianness::Big);
        dmaWriter.Write(dmaEntryCount);

        for (size_t i = 0; i < fileListLines.size(); i++)
        {
            auto fileName = StringHelper::Strip(fileListLines[i], "\r");
            const int romOffset = romVersion.offset + (16 * i);

            const int virtStart = BitConverter::ToInt32BE(romData, romOffset + 0);
            const int virtEnd = BitConverter::ToInt32BE(romData, romOffset + 4);
            const int physStart = BitConverter::ToInt32BE(romData, romOffset + 8);
            const int physEnd = BitConverter::ToInt32BE(romData, romOffset + 12);

            if (physEnd == 0xFFFFFFFF && physStart == 0xFFFFFFFF) {
                continue;
            }

            dmaWriter.Write((uint32_t)(virtEnd - virtStart));
            dmaWriter.Write(fileName);
        }

        dmaWriter.Close();

        printf("Adding DMA file sizes (%u entries).\n", dmaEntryCount);
        auto dmaStreamBuffer = dmaStream->ToVector();
        archive->AddFile("misc/dma_sizes", dmaStreamBuffer.data(), dmaStream->GetLength());

        // Export actor and effect overlay VRAM sizes from the code segment's overlay tables.
        // Find the tables dynamically by searching for known DMA entries within the code segment.
        auto codeData = rom.GetFile("code");

        // Look up a DMA file's VROM start/end by name from the DMA table in romData.
        auto getDmaVromRange = [&](const std::string& targetName, uint32_t& outStart, uint32_t& outEnd) -> bool
        {
            for (size_t i = 0; i < fileListLines.size(); i++)
            {
                if (auto name = StringHelper::Strip(fileListLines[i], "\r"); name == targetName)
                {
                    const int romOff = romVersion.offset + 16 * i;
                    outStart = BitConverter::ToUInt32BE(romData, romOff + 0);
                    outEnd = BitConverter::ToUInt32BE(romData, romOff + 4);
                    return true;
                }
            }
            return false;
        };

        // Search codeData for two consecutive big-endian u32 values (the vromStart/vromEnd pattern).
        auto findTableInCode = [&](const uint32_t vromStart, const uint32_t vromEnd) -> int
        {
            uint8_t needle[8];
            needle[0] = vromStart >> 24 & 0xFF;
            needle[1] = vromStart >> 16 & 0xFF;
            needle[2] = vromStart >>  8 & 0xFF;
            needle[3] = vromStart >>  0 & 0xFF;
            needle[4] = vromEnd >> 24 & 0xFF;
            needle[5] = vromEnd >> 16 & 0xFF;
            needle[6] = vromEnd >>  8 & 0xFF;
            needle[7] = vromEnd >>  0 & 0xFF;

            for (size_t j = 0; j + 8 <= codeData.size(); j += 4)
            {
                if (memcmp(codeData.data() + j, needle, 8) == 0)
                    return (int)j;
            }
            return -1;
        };

        // Actor overlay table: Search for ovl_En_Test's VROM range (actor ID 2). Entry 0 (ovl_player_actor) has null
        // VROM/VRAM because it's linked into the code segment, and its VROM bytes at 0xED410 are gPlayerActorOverlay
        // (a separate struct), not the table.
        auto* actorStream = new MemoryStream();
        BinaryWriter actorWriter(actorStream);
        std::vector<char> actorStreamBuffer;

        uint32_t enTestVromEnd = 0;
        if (uint32_t enTestVromStart = 0; codeData.size() > 0 && getDmaVromRange(
            "ovl_En_Test", enTestVromStart, enTestVromEnd))
        {
            constexpr int enTestActorId = 2;
            if (int enTestOffset = findTableInCode(enTestVromStart, enTestVromEnd);
                enTestOffset >= 0 && enTestOffset >= enTestActorId * 0x20)
            {
                int actorTableStart = enTestOffset - enTestActorId * 0x20;
                auto actorCount = (uint32_t)Globals::Instance->cfg.actorList.size();

                actorWriter.SetEndianness(Endianness::Big);
                actorWriter.Write(actorCount);

                for (uint32_t i = 0; i < actorCount; i++)
                {
                    uint32_t entryOffset = actorTableStart + i * 0x20;
                    uint32_t vramStart = BitConverter::ToUInt32BE(codeData, entryOffset + 0x08);
                    uint32_t vramEnd = BitConverter::ToUInt32BE(codeData, entryOffset + 0x0C);
                    actorWriter.Write(vramEnd - vramStart);
                }

                actorWriter.Close();
                printf("Adding actor overlay VRAM sizes (%u entries).\n", actorCount);
                actorStreamBuffer = actorStream->ToVector();
                archive->AddFile("misc/actor_overlay_sizes", actorStreamBuffer.data(),
                                 actorStream->GetLength());
            }
            else
            {
                printf("Warning: Could not find actor overlay table in code segment.\n");
            }
        }

        // Effect overlay table: Search for ovl_Effect_Ss_Dust's VROM range (effect type 0 = first entry).
        auto* effectStream = new MemoryStream();
        BinaryWriter effectWriter(effectStream);
        std::vector<char> effectStreamBuffer;

        uint32_t dustVromEnd = 0;
        if (uint32_t dustVromStart = 0; !codeData.empty() && getDmaVromRange(
            "ovl_Effect_Ss_Dust", dustVromStart, dustVromEnd))
        {
            if (int effectTableStart = findTableInCode(dustVromStart, dustVromEnd); effectTableStart >= 0)
            {
                uint32_t effectCount = 37; // EFFECT_SS_TYPE_MAX: constant across all OoT versions

                effectWriter.SetEndianness(Endianness::Big);
                effectWriter.Write(effectCount);

                for (uint32_t i = 0; i < effectCount; i++)
                {
                    uint32_t entryOffset = effectTableStart + i * 0x1C;
                    uint32_t vramStart = BitConverter::ToUInt32BE(codeData, entryOffset + 0x08);
                    uint32_t vramEnd = BitConverter::ToUInt32BE(codeData, entryOffset + 0x0C);
                    effectWriter.Write(vramEnd - vramStart);
                }

                effectWriter.Close();
                printf("Adding effect overlay VRAM sizes (%u entries).\n", effectCount);
                effectStreamBuffer = effectStream->ToVector();
                archive->AddFile("misc/effect_overlay_sizes", effectStreamBuffer.data(),
                                 effectStream->GetLength());
            }
            else
            {
                printf("Warning: Could not find effect overlay table in code segment.\n");
            }
        }

        for (const auto& item : files)
        {
            std::string fName = item.first;
            if (fName.find("gTitleZeldaShieldLogoMQTex") != std::string::npos && !rom.IsMQ())
            {
                size_t pos = 0;
                if ((pos = fName.find("gTitleZeldaShieldLogoMQTex", 0)) != std::string::npos)
                {
                    fName.replace(pos, 27, "gTitleZeldaShieldLogoTex");
                }
            }
            const auto& fileData = item.second;
            archive->AddFile(fName, (void*)fileData.data(),	fileData.size());
        }

        archive = nullptr;
    }

    delete fileWriter;
    files.clear();

    // Generate custom otr file for extra assets
    if (customAssetsPath == "" || customArchiveFileName == "" || DiskFile::Exists(customArchiveFileName)) {
        printf("No Custom Assets path or otr file name provided, otr file already exists. Nothing to do.\n");
        return;
    }

    if (!customAssetsPath.ends_with("/")) {
        customAssetsPath += "/";
    }

    const auto& lst = Directory::ListFiles(customAssetsPath);

    printf("Generating Custom OTR Archive...\n");
    auto customOtr = std::make_unique<ExporterArchiveO2R>(customArchiveFileName, true);
    customOtr->CreateArchive(40000);

    printf("Adding portVersion file.\n");
    auto portVersionStreamBuffer = portVersionStream->ToVector();
    customOtr->AddFile("portVersion", (void*)portVersionStreamBuffer.data(), portVersionStream->GetLength());

    std::vector<Data> dataVec;
    std::vector<DataU> dataVec2;


    for (const auto& item : lst)
    {
        size_t filenameSepAt = item.find_last_of("/\\");
        const std::string filename = item.substr(filenameSepAt + 1);

        if (std::count(filename.begin(), filename.end(), '.') >= 2)
        {
            size_t extensionSepAt = filename.find_last_of(".");
            size_t formatSepAt = filename.find_last_of(".", extensionSepAt - 1);

            const std::string extension = filename.substr(extensionSepAt + 1);
            const std::string format = filename.substr(formatSepAt + 1, extensionSepAt - formatSepAt - 1);
            std::string afterPath = item.substr(0, filenameSepAt + formatSepAt + 1);

            if (extension == "png" && (format == "rgba32" || format == "rgb5a1" || format == "i4" || format == "i8" || format == "ia4" || format == "ia8" || format == "ia16" || format == "ci4" || format == "ci8"))
            {
                ZTexture tex(nullptr);
                Globals::Instance->buildRawTexture = true;
                tex.FromPNG(item, ZTexture::GetTextureTypeFromString(format));
                printf("customOtr->AddFile(%s)\n", StringHelper::Split(afterPath, customAssetsPath)[1].c_str());

                OTRExporter_Texture exporter;

                MemoryStream* stream = new MemoryStream();
                BinaryWriter writer(stream);

                exporter.Save(&tex, "", &writer);

                std::string src = tex.GetBodySourceCode();
                writer.Write((char *)src.c_str(), src.size());

                std::vector<char> fileData = stream->ToVector();
                dataVec.push_back({ fileData, StringHelper::Split(afterPath, customAssetsPath)[1], fileData.size() });
                continue;
            }
        }

        if (item.find("accessibility") != std::string::npos)
        {
            std::string extension = filename.substr(filename.find_last_of(".") + 1);
            if (extension == "json")
            {
                const auto &fileData = DiskFile::ReadAllBytes(item);
                printf("Adding accessibility texts %s\n", StringHelper::Split(item, customAssetsPath)[1].c_str());
                dataVec2.push_back({fileData,
                                     StringHelper::Split(item, customAssetsPath)[1], fileData.size() });
            }
            continue;
        }

        const auto& fileData = DiskFile::ReadAllBytes(item);
        printf("customOtr->AddFile(%s)\n", StringHelper::Split(item, customAssetsPath)[1].c_str());
        dataVec2.push_back({ fileData, StringHelper::Split(item, customAssetsPath)[1], fileData.size() });
    }
    for (auto& d : dataVec) {
        customOtr->AddFile(d.filePath, d.fileData.data(), d.size);
    }

    for (auto& d : dataVec2) {
        customOtr->AddFile(d.filePath, d.fileData.data(), d.size);
    }

    printf("Done\n");
    // For O2Rs the zip file MUST be closed while the vectors are still valid so we need to close the file in this function.
    customOtr = nullptr;
}

static void ExporterParseArgs(int argc, char* argv[], int& i)
{
    std::string arg = argv[i];

    if (arg == "--otrfile") {
        archiveFileName = argv[i + 1];
        i++;
    } else if (arg == "--customOtrFile") {
        customArchiveFileName = argv[i + 1];
        i++;
    } else if (arg == "--customAssetsPath") {
        customAssetsPath = argv[i + 1];
        i++;
    } else if (arg == "--portVer") {
        portVersionString = argv[i + 1];
        i++;
    }
}

static bool ExporterProcessFileMode(ZFileMode fileMode)
{
    // Do whatever work is associated with these custom file modes...
    // Return true to indicate one of our own file modes is being processed
    if (fileMode == (ZFileMode)ExporterFileMode::BuildOTR)
        return true;

    return false;
}

static void ExporterFileBegin(ZFile* file)
{
    fileStart = std::chrono::steady_clock::now();

    MemoryStream* stream = new MemoryStream();
    fileWriter = new BinaryWriter(stream);
}

static void ExporterFileEnd(ZFile* file)
{
    // delete fileWriter;
}

static void ExporterResourceEnd(ZResource* res, BinaryWriter& writer)
{
    auto streamShared = writer.GetStream();
    MemoryStream* strem = (MemoryStream*)streamShared.get();

    auto start = std::chrono::steady_clock::now();

    if (res->GetName() != "")
    {
        std::string oName = res->parent->GetOutName();
        std::string rName = res->GetName();
        std::string prefix = OTRExporter_DisplayList::GetPrefix(res);

        //auto xmlFilePath = res->parent->GetXmlFilePath();
        //prefix = StringHelper::Split(StringHelper::Split(xmlFilePath.string(), "xml\\")[1], ".xml")[0];

        if (StringHelper::Contains(oName, "_scene"))
        {
            auto split = StringHelper::Split(oName, "_");
            oName = "";
            for (size_t i = 0; i < split.size() - 1; i++)
                oName += split[i] + "_";

            oName += "scene";
        }
        else if (StringHelper::Contains(oName, "_room"))
        {
            if (Globals::Instance->game != ZGame::MM_RETAIL)
                oName = StringHelper::Split(oName, "_room")[0] + "_scene";
            else
                oName = StringHelper::Split(oName, "_room")[0];
        }

        std::string fName = "";

        if (prefix != "")
            fName = StringHelper::Sprintf("%s/%s/%s", prefix.c_str(), oName.c_str(), rName.c_str());
        else
            fName = StringHelper::Sprintf("%s/%s", oName.c_str(), rName.c_str());

        if (Globals::Instance->fileMode == ZFileMode::ExtractDirectory)
        {
            std::unique_lock Lock(fileMutex);
            files[fName] = strem->ToVector();
        }
        else
            DiskFile::WriteAllBytes("Extract/" + fName, strem->ToVector());
    }

    auto end = std::chrono::steady_clock::now();
    size_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    //if (diff > 10)
        //printf("Exported Resource End %s in %zums\n", res->GetName().c_str(), diff);
}

static void ExporterProcessCompilable(tinyxml2::XMLElement* reader)
{
    std::string nodeName = reader->Name();
}

static void ExporterXMLBegin()
{
}

static void ExporterXMLEnd()
{
}

void AddFile(std::string fName, std::vector<char> data)
{
    if (Globals::Instance->fileMode != ZFileMode::ExtractDirectory)
        DiskFile::WriteAllBytes("Extract/" + fName, data);
    else
    {
        std::unique_lock Lock(fileMutex);
        files[fName] = data;
    }
}

void ImportExporters()
{
    // In this example we set up a new exporter called "EXAMPLE".
    // By running ZAPD with the argument -se EXAMPLE, we tell it that we want to use this exporter for our resources.
    ExporterSet* exporterSet = new ExporterSet();
    exporterSet->processFileModeFunc = ExporterProcessFileMode;
    exporterSet->parseFileModeFunc = ExporterParseFileMode;
    exporterSet->processCompilableFunc = ExporterProcessCompilable;
    exporterSet->parseArgsFunc = ExporterParseArgs;
    exporterSet->beginFileFunc = ExporterFileBegin;
    exporterSet->endFileFunc = ExporterFileEnd;
    exporterSet->beginXMLFunc = ExporterXMLBegin;
    exporterSet->endXMLFunc = ExporterXMLEnd;
    exporterSet->resSaveFunc = ExporterResourceEnd;
    exporterSet->endProgramFunc = ExporterProgramEnd;

    exporterSet->exporters[ZResourceType::Background] = new OTRExporter_Background();
    exporterSet->exporters[ZResourceType::Texture] = new OTRExporter_Texture();
    exporterSet->exporters[ZResourceType::Room] = new OTRExporter_Room();
    exporterSet->exporters[ZResourceType::AltHeader] = new OTRExporter_Room();
    exporterSet->exporters[ZResourceType::Scene] = new OTRExporter_Room();
    exporterSet->exporters[ZResourceType::CollisionHeader] = new OTRExporter_Collision();
    exporterSet->exporters[ZResourceType::DisplayList] = new OTRExporter_DisplayList();
    exporterSet->exporters[ZResourceType::PlayerAnimationData] = new OTRExporter_PlayerAnimationExporter();
    exporterSet->exporters[ZResourceType::Skeleton] = new OTRExporter_Skeleton();
    exporterSet->exporters[ZResourceType::Limb] = new OTRExporter_SkeletonLimb();
    exporterSet->exporters[ZResourceType::Animation] = new OTRExporter_Animation();
    exporterSet->exporters[ZResourceType::Cutscene] = new OTRExporter_Cutscene();
    exporterSet->exporters[ZResourceType::Vertex] = new OTRExporter_Vtx();
    exporterSet->exporters[ZResourceType::Array] = new OTRExporter_Array();
    exporterSet->exporters[ZResourceType::Path] = new OTRExporter_Path();
    exporterSet->exporters[ZResourceType::Text] = new OTRExporter_Text();
#ifdef GAME_MM
    exporterSet->exporters[ZResourceType::TextMM] = new OTRExporter_TextMM();
    exporterSet->exporters[ZResourceType::KeyFrameSkel] = new OTRExporter_CKeyFrameSkel();
    exporterSet->exporters[ZResourceType::KeyFrameAnimation] = new OTRExporter_CKeyFrameAnim();
    exporterSet->exporters[ZResourceType::TextureAnimation] = new OTRExporter_TextureAnimation();
#endif
    exporterSet->exporters[ZResourceType::Blob] = new OTRExporter_Blob();
    exporterSet->exporters[ZResourceType::Mtx] = new OTRExporter_MtxExporter();
    exporterSet->exporters[ZResourceType::Audio] = new OTRExporter_Audio();
    Globals::AddExporter("OTR", exporterSet);

    InitVersionInfo();
}
