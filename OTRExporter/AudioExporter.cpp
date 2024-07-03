#include "AudioExporter.h"
#include "Main.h"
#include <Utils/MemoryStream.h>
#include <Globals.h>
#include <Utils/DiskFile.h>
#include "DisplayListExporter.h"

const char* OTRExporter_Audio::GetMediumStr(uint8_t medium) {
    switch (medium) {
        case 0:
            return "Ram";
        case 1:
            return "Unk";
        case 2:
            return "Cart";
        case 3:
            return "Disk";
        case 5:
            return "RamUnloaded";
        default:
            return "ERROR";
    }
}

const char* OTRExporter_Audio::GetCachePolicyStr(uint8_t policy) {
    switch (policy) {
        case 0:
            return "Temporary";
        case 1:
            return "Persistent";
        case 2:
            return "Either";
        case 3:
            return "Permanent";
        default:
            return "ERROR";
    }
}

std::string OTRExporter_Audio::GetSampleEntryReference(ZAudio* audio, SampleEntry* entry, std::map<uint32_t, SampleEntry*> samples)
{

    if (entry != nullptr)
    {
        if (audio->sampleOffsets[entry->bankId].find(entry->sampleLoopOffset) != audio->sampleOffsets[entry->bankId].end())
        {
            if (audio->sampleOffsets[entry->bankId][entry->sampleLoopOffset].find(entry->sampleDataOffset) != audio->sampleOffsets[entry->bankId][entry->sampleLoopOffset].end())
            {
                return(StringHelper::Sprintf("audio/samples/%s", audio->sampleOffsets[entry->bankId][entry->sampleLoopOffset][entry->sampleDataOffset].c_str()));
            }
            else
                return(entry->fileName);
        }
        else
            return(entry->fileName);
    }
    else
        return("");
}

void OTRExporter_Audio::WriteSampleEntry(SampleEntry* entry, BinaryWriter* writer)
{
    WriteHeader(nullptr, "", writer, static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSample), 2);

    writer->Write(entry->codec);
    writer->Write(entry->medium);
    writer->Write(entry->unk_bit26);
    writer->Write(entry->unk_bit25);

    writer->Write((uint32_t)entry->data.size());
    writer->Write((char*)entry->data.data(), entry->data.size());

    writer->Write((uint32_t)(entry->loop.start));
    writer->Write((uint32_t)(entry->loop.end));
    writer->Write((uint32_t)(entry->loop.count));
    writer->Write((uint32_t)entry->loop.states.size());

    for (size_t i = 0; i < entry->loop.states.size(); i++)
        writer->Write((entry->loop.states[i]));

    writer->Write((uint32_t)(entry->book.order));
    writer->Write((uint32_t)(entry->book.npredictors));
    writer->Write((uint32_t)entry->book.books.size());

    for (size_t i = 0; i < entry->book.books.size(); i++)
        writer->Write((entry->book.books[i]));
}



void OTRExporter_Audio::WriteSoundFontEntry(ZAudio* audio, SoundFontEntry* entry, std::map<uint32_t, SampleEntry*> samples, BinaryWriter* writer)
{
    writer->Write((uint8_t)(entry != nullptr ? 1 : 0));

    if (entry != nullptr)
    {
        writer->Write(GetSampleEntryReference(audio, entry->sampleEntry, samples));
        writer->Write(entry->tuning);
    }
}

void OTRExporter_Audio::WriteSoundFontEntry(ZAudio* audio, SoundFontEntry* entry,
                                            std::map<uint32_t, SampleEntry*> samples, tinyxml2::XMLElement* xmlDoc,
                                            const char* name) {
    tinyxml2::XMLElement* sfEntry = xmlDoc->InsertNewChildElement(name);

    if (entry != nullptr) {
    sfEntry->SetAttribute("SampleRef", GetSampleEntryReference(audio, entry->sampleEntry, samples).c_str());
    sfEntry->SetAttribute("Tuning", entry->tuning);
    

    }
    xmlDoc->InsertEndChild(sfEntry);
}

void OTRExporter_Audio::WriteEnvData(std::vector<AdsrEnvelope*> envelopes, BinaryWriter* writer)
{
    writer->Write((uint32_t)envelopes.size());

    for (auto env : envelopes)
    {
        writer->Write(env->delay);
        writer->Write(env->arg);
    }
}

void OTRExporter_Audio::WriteEnvData(std::vector<AdsrEnvelope*> envelopes, tinyxml2::XMLElement* xmlDoc) {
    tinyxml2::XMLElement* envs = xmlDoc->InsertNewChildElement("Envelopes");
    envs->SetAttribute("Count", (uint32_t)envelopes.size());

    for (auto e : envelopes) {
        tinyxml2::XMLElement* env = envs->InsertNewChildElement("Envelope");
        env->SetAttribute("Delay", e->delay);
        env->SetAttribute("Arg", e->arg);
    }
    xmlDoc->InsertEndChild(envs);
}


void OTRExporter_Audio::WriteSoundFontTableXML(ZAudio* audio) {
    for (size_t i = 0; i < audio->soundFontTable.size(); i++) {
        tinyxml2::XMLDocument soundFont;
        tinyxml2::XMLElement* root = soundFont.NewElement("SoundFont");
        root->SetAttribute("Version", 0);
        root->SetAttribute("Num", (uint32_t)i);
        root->SetAttribute("Medium", GetMediumStr(audio->soundFontTable[i].medium));
        root->SetAttribute("CachePolicy", GetCachePolicyStr(audio->soundFontTable[i].cachePolicy));
        root->SetAttribute("Data1", audio->soundFontTable[i].data1);
        root->SetAttribute("Data2", audio->soundFontTable[i].data2);
        root->SetAttribute("Data3", audio->soundFontTable[i].data3);
        soundFont.InsertFirstChild(root);
        
        tinyxml2::XMLElement* drums = root->InsertNewChildElement("Drums");
        drums->SetAttribute("Count", (uint32_t)audio->soundFontTable[i].drums.size());

        for (const auto& d : audio->soundFontTable[i].drums) {
            tinyxml2::XMLElement* drum = drums->InsertNewChildElement("Drum");
            drum->SetAttribute("ReleaseRate", d.releaseRate);
            drum->SetAttribute("Pan", d.pan);
            drum->SetAttribute("Loaded", d.loaded);
            drum->SetAttribute("SampleRef", GetSampleEntryReference(audio, d.sample, audio->samples).c_str());
            drum->SetAttribute("Tuning", d.tuning);

            WriteEnvData(d.env, drum);
            drums->InsertEndChild(drum);
        }
        root->InsertEndChild(drums);

        tinyxml2::XMLElement* instruments = root->InsertNewChildElement("Instruments");
        instruments->SetAttribute("Count", (uint32_t)audio->soundFontTable[i].instruments.size());

        //for (size_t k = 0; k < audio->soundFontTable[i].instruments.size(); k++) {
        for (const auto i : audio->soundFontTable[i].instruments) {
            tinyxml2::XMLElement* instrument = instruments->InsertNewChildElement("Instrument");

            instrument->SetAttribute("IsValid", i.isValidInstrument);
            instrument->SetAttribute("Loaded", i.loaded);
            instrument->SetAttribute("NormalRangeLo", i.normalRangeLo);
            instrument->SetAttribute("NormalRangeHi", i.normalRangeHi);
            instrument->SetAttribute("ReleaseRate", i.releaseRate);

            WriteEnvData(i.env, instrument);

            WriteSoundFontEntry(audio, i.lowNotesSound, audio->samples, instrument, "LowNotesSound");
            WriteSoundFontEntry(audio, i.normalNotesSound, audio->samples, instrument, "NormalNotesSound");
            WriteSoundFontEntry(audio, i.highNotesSound, audio->samples, instrument, "HighNotesSound");
        }
        root->InsertEndChild(instruments);

        tinyxml2::XMLElement* sfxTbl = root->InsertNewChildElement("SfxTable");
        sfxTbl->SetAttribute("Count", (uint32_t)audio->soundFontTable[i].soundEffects.size());

        for (const auto s : audio->soundFontTable[i].soundEffects) {
            WriteSoundFontEntry(audio, s, audio->samples, sfxTbl, "Sfx");
        }
        root->InsertEndChild(sfxTbl);
        soundFont.InsertEndChild(root);
        
        tinyxml2::XMLPrinter printer;
        soundFont.Accept(&printer);
        
        std::string fName = OTRExporter_DisplayList::GetPathToRes(
            (ZResource*)(audio), StringHelper::Sprintf("fonts/%s", audio->soundFontNames[i].c_str()));
        std::vector<char> xmlData((printer.CStr()), printer.CStr() + printer.CStrSize());
        AddFile(fName, xmlData);
    }
}

void OTRExporter_Audio::WriteSoundFontTableBinary(ZAudio* audio) {
    for (size_t i = 0; i < audio->soundFontTable.size(); i++) {
        MemoryStream* fntStream = new MemoryStream();
        BinaryWriter fntWriter = BinaryWriter(fntStream);

        WriteHeader(nullptr, "", &fntWriter, static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSoundFont), 2);

        fntWriter.Write((uint32_t)i);
        fntWriter.Write(audio->soundFontTable[i].medium);
        fntWriter.Write(audio->soundFontTable[i].cachePolicy);
        fntWriter.Write(audio->soundFontTable[i].data1);
        fntWriter.Write(audio->soundFontTable[i].data2);
        fntWriter.Write(audio->soundFontTable[i].data3);

        fntWriter.Write((uint32_t)audio->soundFontTable[i].drums.size());
        fntWriter.Write((uint32_t)audio->soundFontTable[i].instruments.size());
        fntWriter.Write((uint32_t)audio->soundFontTable[i].soundEffects.size());

        for (size_t k = 0; k < audio->soundFontTable[i].drums.size(); k++) {
            fntWriter.Write(audio->soundFontTable[i].drums[k].releaseRate);
            fntWriter.Write(audio->soundFontTable[i].drums[k].pan);
            fntWriter.Write(audio->soundFontTable[i].drums[k].loaded);

            WriteEnvData(audio->soundFontTable[i].drums[k].env, &fntWriter);
            fntWriter.Write((uint8_t)(audio->soundFontTable[i].drums[k].sample != nullptr ? 1 : 0));

            fntWriter.Write(GetSampleEntryReference(audio, audio->soundFontTable[i].drums[k].sample, audio->samples));
            fntWriter.Write(audio->soundFontTable[i].drums[k].tuning);
        }

        for (size_t k = 0; k < audio->soundFontTable[i].instruments.size(); k++) {
            fntWriter.Write((uint8_t)audio->soundFontTable[i].instruments[k].isValidInstrument);

            fntWriter.Write(audio->soundFontTable[i].instruments[k].loaded);
            fntWriter.Write(audio->soundFontTable[i].instruments[k].normalRangeLo);
            fntWriter.Write(audio->soundFontTable[i].instruments[k].normalRangeHi);
            fntWriter.Write(audio->soundFontTable[i].instruments[k].releaseRate);

            WriteEnvData(audio->soundFontTable[i].instruments[k].env, &fntWriter);

            WriteSoundFontEntry(audio, audio->soundFontTable[i].instruments[k].lowNotesSound, audio->samples,
                                &fntWriter);
            WriteSoundFontEntry(audio, audio->soundFontTable[i].instruments[k].normalNotesSound, audio->samples,
                                &fntWriter);
            WriteSoundFontEntry(audio, audio->soundFontTable[i].instruments[k].highNotesSound, audio->samples,
                                &fntWriter);
        }

        for (size_t k = 0; k < audio->soundFontTable[i].soundEffects.size(); k++) {
            WriteSoundFontEntry(audio, audio->soundFontTable[i].soundEffects[k], audio->samples, &fntWriter);
        }

        // std::string fName = OTRExporter_DisplayList::GetPathToRes(res, StringHelper::Sprintf("fonts/font_%02X", i));
        std::string fName = OTRExporter_DisplayList::GetPathToRes(
            (ZResource*)(audio), StringHelper::Sprintf("fonts/%s", audio->soundFontNames[i].c_str()));
        AddFile(fName, fntStream->ToVector());
    }
}

void OTRExporter_Audio::WriteSequenceXML(ZAudio* audio) {
    for (size_t i = 0; i < audio->sequences.size(); i++) {
        MemoryStream* seqStream = new MemoryStream();
        BinaryWriter seqWriter = BinaryWriter(seqStream);
        auto& seq = audio->sequences[i];

        tinyxml2::XMLDocument sequence;
        tinyxml2::XMLElement* root = sequence.NewElement("Sequence");
        root->SetAttribute("Index", (uint32_t)i);
        root->SetAttribute("Medium", GetMediumStr(audio->sequenceTable[i].medium));
        root->SetAttribute("CachePolicy", GetCachePolicyStr(audio->sequenceTable[i].cachePolicy));
        root->SetAttribute("Size", (uint32_t)seq.size());

        std::string seqName = OTRExporter_DisplayList::GetPathToRes(
            (ZResource*)(audio), StringHelper::Sprintf("sequencedata/%s_RAW", audio->seqNames[i].c_str()));
        root->SetAttribute("Path", seqName.c_str());


        tinyxml2::XMLElement* fontIndicies = root->InsertNewChildElement("FontIndicies");
        for (size_t k = 0; k < audio->fontIndices[i].size(); k++) {
            tinyxml2::XMLElement* fontIndex = fontIndicies->InsertNewChildElement("FontIndex");
            fontIndex->SetAttribute("FontIdx", audio->fontIndices[i][k]);

        }
        root->InsertEndChild(fontIndicies);
        root->InsertEndChild(root);
        sequence.InsertEndChild(root);
        seqWriter.Write(seq.data(), seq.size());
        AddFile(seqName, seqStream->ToVector());

        tinyxml2::XMLPrinter printer;
        sequence.Accept(&printer);
        

        std::string seqMetaName = OTRExporter_DisplayList::GetPathToRes(
            (ZResource*)(audio), StringHelper::Sprintf("sequences/%s_META", audio->seqNames[i].c_str()));
        std::vector<char> xmlData((printer.CStr()), printer.CStr() + printer.CStrSize());
        AddFile(seqMetaName, xmlData);
    }
}

void OTRExporter_Audio::WriteSequenceBinary(ZAudio* audio) {
    for (size_t i = 0; i < audio->sequences.size(); i++) {
        auto& seq = audio->sequences[i];

        MemoryStream* seqStream = new MemoryStream();
        BinaryWriter seqWriter = BinaryWriter(seqStream);

        WriteHeader(nullptr, "", &seqWriter, static_cast<uint32_t>(SOH::ResourceType::SOH_AudioSequence), 2);

        seqWriter.Write((uint32_t)seq.size());
        seqWriter.Write(seq.data(), seq.size());
        seqWriter.Write((uint8_t)i);
        seqWriter.Write((uint8_t)audio->sequenceTable[i].medium);
        seqWriter.Write((uint8_t)audio->sequenceTable[i].cachePolicy);
        seqWriter.Write((uint32_t)audio->fontIndices[i].size());

        for (size_t k = 0; k < audio->fontIndices[i].size(); k++)
            seqWriter.Write((uint8_t)audio->fontIndices[i][k]);

        std::string fName = OTRExporter_DisplayList::GetPathToRes(
            (ZResource*)(audio), StringHelper::Sprintf("sequences/%s", audio->seqNames[i].c_str()));
        AddFile(fName, seqStream->ToVector());
    }
}

void OTRExporter_Audio::Save(ZResource* res, const fs::path& outPath, BinaryWriter* writer)
{
    ZAudio* audio = (ZAudio*)res;

    WriteHeader(res, outPath, writer, static_cast<uint32_t>(SOH::ResourceType::SOH_Audio), 2);

    // Write Samples as individual files
    for (auto pair : audio->samples)
    {
        MemoryStream* sampleStream = new MemoryStream();
        BinaryWriter sampleWriter = BinaryWriter(sampleStream);

        writer->Write((uint32_t)pair.first);
        WriteSampleEntry(pair.second, &sampleWriter);

        std::string basePath = "";

        if (audio->sampleOffsets[pair.second->bankId].find(pair.second->sampleLoopOffset) != audio->sampleOffsets[pair.second->bankId].end())
        {
            if (audio->sampleOffsets[pair.second->bankId][pair.second->sampleLoopOffset].find(pair.second->sampleDataOffset) != audio->sampleOffsets[pair.second->bankId][pair.second->sampleLoopOffset].end())
                basePath = StringHelper::Sprintf("samples/%s", audio->sampleOffsets[pair.second->bankId][pair.second->sampleLoopOffset][pair.second->sampleDataOffset].c_str());
            else
                basePath = StringHelper::Sprintf("samples/sample_%08X", pair.first);
        }
        else
            basePath = StringHelper::Sprintf("samples/sample_%08X", pair.first);

        std::string fName = OTRExporter_DisplayList::GetPathToRes(res, basePath);
        AddFile(fName, sampleStream->ToVector());
    }

    // Write the soundfont table
    WriteSoundFontTableXML(audio);
    
    // Write Sequences
    WriteSequenceXML(audio);
}
