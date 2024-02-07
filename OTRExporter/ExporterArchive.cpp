#include "ExporterArchive.h"
#include "Utils/StringHelper.h"
#include <StrHash64/StrHash64.h>
#include <filesystem>

ExporterArchive::ExporterArchive(const std::string& path, bool enableWriting) : mPath(path) {
    mMpq = nullptr;
    Load(enableWriting);
}


bool ExporterArchive::Load(bool enableWriting) {
    HANDLE mpqHandle = NULL;

    bool baseLoaded = false;
    std::string fullPath = std::filesystem::absolute(mPath).string();
    bool openArchiveSuccess;
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        openArchiveSuccess =
            SFileOpenArchive(fullPath.c_str(), 0, enableWriting ? 0 : MPQ_OPEN_READ_ONLY, &mpqHandle);
    }
    if (openArchiveSuccess) {
        printf("Opened mpq file ");
        printf(fullPath.c_str());
        printf("\n");
        mMpq = mpqHandle;
        mPath = fullPath;

        
        baseLoaded = true;
    }

    if (!baseLoaded) {
        printf("No valid OTR file was provided.");
        return false;
    }

    return true;
}

std::shared_ptr<ExporterArchive> ExporterArchive::CreateArchive(const std::string& archivePath, size_t fileCapacity) {
    auto archive = std::make_shared<ExporterArchive>(archivePath, true);

    TCHAR* fileName = new TCHAR[archivePath.size() + 1];
    fileName[archivePath.size()] = 0;
    std::copy(archivePath.begin(), archivePath.end(), fileName);

    bool success;
    {
        const std::lock_guard<std::mutex> lock(archive->mMutex);
        success = SFileCreateArchive(fileName, MPQ_CREATE_LISTFILE | MPQ_CREATE_ATTRIBUTES | MPQ_CREATE_ARCHIVE_V2,
                                     fileCapacity, &archive->mMpq);
    }
    int32_t error = GetLastError();

    delete[] fileName;

    if (success) {
        return archive;
    } else {
        printf("We tried to create an archive, but it has fallen and cannot get up.\n");
        return nullptr;
    }
}

bool ExporterArchive::AddFile(const std::string& filePath, uintptr_t fileData, DWORD fileSize) {
    HANDLE hFile;
#ifdef _WIN32
    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);
    FILETIME t;
    SystemTimeToFileTime(&sysTime, &t);
    ULONGLONG theTime = static_cast<uint64_t>(t.dwHighDateTime) << (sizeof(t.dwHighDateTime) * 8) | t.dwLowDateTime;
#else
    time_t theTime;
    time(&theTime);
#endif

    std::string updatedPath = filePath;

    StringHelper::ReplaceOriginal(updatedPath, "\\", "/");

    bool createFileSuccess;
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        createFileSuccess =
            SFileCreateFile(mMpq, updatedPath.c_str(), theTime, fileSize, 0, MPQ_FILE_COMPRESS, &hFile);
    }
    if (!createFileSuccess) {
        printf("Failed to create file.\n");
        return false;
    }

    bool writeFileSuccess;
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        writeFileSuccess = SFileWriteFile(hFile, (void*)fileData, fileSize, MPQ_COMPRESSION_ZLIB);
    }
    if (!writeFileSuccess) {
        printf("Failed to write.\n");
        bool closeFileSuccess;
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            closeFileSuccess = SFileCloseFile(hFile);
        }
        if (!closeFileSuccess) {
            printf("Failed to close.\n");
        }
        return false;
    }

    bool finishFileSuccess;
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        finishFileSuccess = SFileFinishFile(hFile);
    }
    if (!finishFileSuccess) {
        printf("Failed to finish file.\n");
        bool closeFileSuccess;
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            closeFileSuccess = SFileCloseFile(hFile);
        }
        if (!closeFileSuccess) {
            printf("Failed to close after finish failure.\n");
        }
        return false;
    }
    // SFileFinishFile already frees the handle, so no need to close it again.

    mAddedFiles.push_back(updatedPath);
    mHashes[CRC64(updatedPath.c_str())] = updatedPath;

    return true;
}