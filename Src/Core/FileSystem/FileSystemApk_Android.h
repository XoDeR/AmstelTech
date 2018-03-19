#pragma once

#include "Core/Platform.h"

#if RIO_PLATFORM_ANDROID

#include "Core/FileSystem/FileSystem.h"

#include <sys/types.h> // off_t
#include <android/asset_manager.h>

namespace Rio
{

// Access files on Android's assets folder
// The assets folder is read-only and all the paths are relative
struct FileSystemApk : public FileSystem
{
	Allocator* allocator = nullptr;
	AAssetManager* assetManager = nullptr;

	FileSystemApk(Allocator& a, AAssetManager* assetManager);
	File* open(const char* path, FileOpenMode::Enum mode);
	void close(File& file);
	bool exists(const char* path);
	bool getIsDirectory(const char* path);
	bool getIsFile(const char* path);
	uint64_t getLastModifiedTime(const char* path);
	void createDirectory(const char* path);
	void deleteDirectory(const char* path);
	void deleteFile(const char* path);
	void getFileList(const char* path, Vector<DynamicString>& files);
	void getAbsolutePath(const char* path, DynamicString& osPath);
};

} // namespace Rio

#endif // RIO_PLATFORM_ANDROID
