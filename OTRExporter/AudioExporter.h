#pragma once

#include "ZResource.h"
#include "ZAudio.h"
#include "Exporter.h"
#include <Utils/BinaryWriter.h>

class OTRExporter_Audio : public OTRExporter
{
public:
    void WriteSampleEntry(SampleEntry* entry, BinaryWriter* writer);
    virtual void Save(ZResource* res, const fs::path& outPath, BinaryWriter* writer) override;

private:
    void WriteSoundFontTableBinary(ZAudio* audio);
    void WriteSoundFontTableXML(ZAudio* audio);
    void WriteSequenceBinary(ZAudio* audio);
    void WriteSequenceXML(ZAudio* audio);
    std::string GetSampleEntryReference(ZAudio* audio, SampleEntry* entry, std::map<uint32_t, SampleEntry*> samples);
    void WriteEnvData(std::vector<AdsrEnvelope*> envelopes, BinaryWriter* writer);
    void WriteEnvData(std::vector<AdsrEnvelope*> envelopes, tinyxml2::XMLElement* xmlDoc);
    void WriteSoundFontEntry(ZAudio* audio, SoundFontEntry* entry, std::map<uint32_t, SampleEntry*> samples,
                             BinaryWriter* writer);
    void WriteSoundFontEntry(ZAudio* audio, SoundFontEntry* entry, std::map<uint32_t, SampleEntry*> samples,
                             tinyxml2::XMLElement* xmlDoc, const char* name);
    const char* GetMediumStr(uint8_t medium);
    const char* GetCachePolicyStr(uint8_t policy);

};