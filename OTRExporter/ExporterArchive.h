#pragma once

#undef _DLL

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <StormLib.h>

class ExporterArchive : public std::enable_shared_from_this<ExporterArchive> {
  public:
    ExporterArchive(const std::string& path, bool enableWriting);
    ~ExporterArchive();

    static std::shared_ptr<ExporterArchive> CreateArchive(const std::string& archivePath, size_t fileCapacity);
    bool AddFile(const std::string& filePath, uintptr_t fileData, DWORD fileSize);

  private:
    std::string mPath;
    HANDLE mMpq;
    std::mutex mMutex;
    std::vector<std::string> mAddedFiles;
    std::unordered_map<uint64_t, std::string> mHashes;

    bool Load(bool enableWriting);
    bool Unload();
};
