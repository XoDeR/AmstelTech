#include "Core/FileSystem/FileSystemApk_Android.h"

#if RIO_PLATFORM_ANDROID

#include "Core/Config.h"

#include "Core/Containers/Vector.h"
#include "Core/FileSystem/File.h"

#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Strings/DynamicString.h"

namespace Rio
{

struct FileApk : public File
{
	AAssetManager* assetManager = nullptr;
	AAsset* asset = nullptr;

	FileApk(AAssetManager* assetManager)
		: assetManager(assetManager)
	{
	}

	virtual ~FileApk()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum /*mode*/)
	{
		asset = AAssetManager_open(assetManager, path, AASSET_MODE_RANDOM);
		RIO_ASSERT(asset != nullptr, "AAssetManager_open: failed to open %s", path);
	}

	void close()
	{
		if (asset)
		{
			AAsset_close(asset);
			asset = nullptr;
		}
	}

	uint32_t getFileSize()
	{
		return AAsset_getLength(asset);
	}

	uint32_t getFilePosition()
	{
		return uint32_t(AAsset_getLength(asset) - AAsset_getRemainingLength(asset));
	}

	bool getIsEndOfFile()
	{
		return AAsset_getRemainingLength(asset) == 0;
	}

	void seek(uint32_t position)
	{
		off_t seekResult = AAsset_seek(asset, (off_t)position, SEEK_SET);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	void seekToEnd()
	{
		off_t seekResult = AAsset_seek(asset, 0, SEEK_END);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	void skip(uint32_t bytes)
	{
		off_t seekResult = AAsset_seek(asset, (off_t)bytes, SEEK_CUR);
		RIO_ASSERT(seekResult != (off_t)-1, "AAsset_seek: error");
		RIO_UNUSED(seekResult);
	}

	uint32_t read(void* data, uint32_t size)
	{
		RIO_ENSURE(nullptr != data);
		return (uint32_t)AAsset_read(asset, data, size);
	}

	uint32_t write(const void* /*data*/, uint32_t /*size*/)
	{
		RIO_FATAL("Apk files are read only!");
		return 0;
	}

	void flush()
	{
		// Not needed
	}
};

FileSystemApk::FileSystemApk(Allocator& a, AAssetManager* assetManager)
	: allocator(&a)
	, assetManager(assetManager)
{
}

File* FileSystemApk::open(const char* path, FileOpenMode::Enum mode)
{
	RIO_ENSURE(nullptr != path);
	RIO_ASSERT(mode == FileOpenMode::READ, "Cannot open for writing in Android assets folder");
	FileApk* file = RIO_NEW(*allocator, FileApk)(assetManager);
	file->open(path, mode);
	return file;
}

void FileSystemApk::close(File& file)
{
	RIO_DELETE(*allocator, &file);
}

bool FileSystemApk::exists(const char* /*path*/)
{
	return true;
}

bool FileSystemApk::getIsDirectory(const char* path)
{
	return true;
}

bool FileSystemApk::getIsFile(const char* path)
{
	return true;
}

uint64_t FileSystemApk::getLastModifiedTime(const char* path)
{
	return 0;
}

void FileSystemApk::createDirectory(const char* /*path*/)
{
	RIO_FATAL("Cannot create directory in Android assets folder");
}

void FileSystemApk::deleteDirectory(const char* /*path*/)
{
	RIO_FATAL("Cannot delete directory in Android assets folder");
}

void FileSystemApk::deleteFile(const char* /*path*/)
{
	RIO_FATAL("Cannot delete file in Android assets folder");
}

void FileSystemApk::getFileList(const char* path, Vector<DynamicString>& files)
{
	RIO_ENSURE(nullptr != path);

	AAssetDir* rootDirectory = AAssetManager_openDir(assetManager, path);
	RIO_ASSERT(rootDirectory != nullptr, "Failed to open Android assets folder");

	const char* fileName = nullptr;
	while ((fileName = AAssetDir_getNextFileName(rootDirectory)) != nullptr)
	{
		TempAllocator512 ta;
		DynamicString name(ta);
		name = fileName;
		VectorFn::pushBack(files, name);
	}

	AAssetDir_close(rootDirectory);
}

void FileSystemApk::getAbsolutePath(const char* path, DynamicString& osPath)
{
	osPath = path;
}

} // namespace Rio

#endif // RIO_PLATFORM_ANDROID
