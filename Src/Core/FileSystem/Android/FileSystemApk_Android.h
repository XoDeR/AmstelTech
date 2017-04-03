// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Platform.h"

#if RIO_PLATFORM_ANDROID

#include "Core/FileSystem/FileSystem.h"

#include <sys/types.h> // off_t
#include <android/assetManager.h>

namespace RioCore
{

// Access files on Android's assets folder
// The assets folder is read-only and all the paths are relative
class FileSystemApk : public FileSystem
{
public:
	FileSystemApk(Allocator& a, AAssetManager* assetManager);
	File* open(const char* path, FileOpenMode::Enum mode);
	void close(File& file);
	bool getDoesExist(const char* path);
	bool getIsDirectory(const char* path);
	bool getIsFile(const char* path);
	uint64_t getLastModifiedTime(const char* path);
	void createDirectory(const char* path);
	void deleteDirectory(const char* path);
	void createFile(const char* path);
	void deleteFile(const char* path);
	void getFileList(const char* path, Vector<DynamicString>& files);
	void getAbsolutePath(const char* path, DynamicString& osPath);
private:
	Allocator* allocator;
	AAssetManager* assetManager;
};

} // namespace RioCore

#endif // RIO_PLATFORM_ANDROID
// Copyright (c) 2016 Volodymyr Syvochka