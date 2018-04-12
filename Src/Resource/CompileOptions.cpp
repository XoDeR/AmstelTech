#include "Resource/CompileOptions.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/Path.h"
#include "Core/Guid.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"

#include "Device/DeviceLog.h"

#include "Resource/DataCompiler.h"

namespace Rio
{

CompileOptions::CompileOptions(DataCompiler& dataCompiler, FileSystem& dataFileSystem, DynamicString& sourcePath, Buffer& outputBuffer, const char* platformName)
	: dataCompiler(dataCompiler)
	, dataFileSystem(dataFileSystem)
	, sourcePath(sourcePath)
	, outputBuffer(outputBuffer)
	, platformName(platformName)
	, dependencyList(getDefaultAllocator())
{
}

void CompileOptions::error(const char* message, va_list argumentList)
{
	dataCompiler.error(message, argumentList);
}

void CompileOptions::error(const char* message, ...)
{
	va_list argumentList;
	va_start(argumentList, message);
	error(message, argumentList);
	va_end(argumentList);
}

const char* CompileOptions::getSourcePath()
{
	return sourcePath.getCStr();
}

bool CompileOptions::doesFileExist(const char* path)
{
	TempAllocator256 tempAllocator256;
	DynamicString sourceDirectory(tempAllocator256);
	FileSystemDisk fileSystemDisk(tempAllocator256);

	dataCompiler.getSourceDirectory(path, sourceDirectory);
	fileSystemDisk.setPrefix(sourceDirectory.getCStr());

	return fileSystemDisk.exists(path);
}

bool CompileOptions::doesResourceExist(const char* type, const char* name)
{
	TempAllocator1024 tempAllocator1024;
	DynamicString path(tempAllocator1024);
	path += name;
	path += ".";
	path += type;
	return doesFileExist(path.getCStr());
}

Buffer CompileOptions::readTemporary(const char* path)
{
	File* file = dataFileSystem.open(path, FileOpenMode::READ);
	uint32_t size = file->getFileSize();
	Buffer buffer(getDefaultAllocator());
	ArrayFn::resize(buffer, size);
	file->read(ArrayFn::begin(buffer), size);
	dataFileSystem.close(*file);
	return buffer;
}

void CompileOptions::writeTemporary(const char* path, const char* data, uint32_t size)
{
	File* file = dataFileSystem.open(path, FileOpenMode::WRITE);
	file->write(data, size);
	dataFileSystem.close(*file);
}

void CompileOptions::writeTemporary(const char* path, const Buffer& data)
{
	writeTemporary(path, ArrayFn::begin(data), ArrayFn::getCount(data));
}

Buffer CompileOptions::read()
{
	return read(sourcePath.getCStr());
}

Buffer CompileOptions::read(const char* path)
{
	addDependency(path);

	TempAllocator256 tempAllocator256;
	DynamicString sourceDirectory(tempAllocator256);
	dataCompiler.getSourceDirectory(path, sourceDirectory);

	FileSystemDisk sourceFileSystem(tempAllocator256);
	sourceFileSystem.setPrefix(sourceDirectory.getCStr());

	File* file = sourceFileSystem.open(path, FileOpenMode::READ);
	const uint32_t size = file->getFileSize();
	Buffer buffer(getDefaultAllocator());
	ArrayFn::resize(buffer, size);
	file->read(ArrayFn::begin(buffer), size);
	sourceFileSystem.close(*file);
	return buffer;
}

void CompileOptions::getAbsolutePath(const char* path, DynamicString& absolutePath)
{
	TempAllocator256 tempAllocator256;
	DynamicString sourceDirectory(tempAllocator256);
	dataCompiler.getSourceDirectory(path, sourceDirectory);

	FileSystemDisk sourceFileSystem(tempAllocator256);
	sourceFileSystem.setPrefix(sourceDirectory.getCStr());
	sourceFileSystem.getAbsolutePath(path, absolutePath);
}

void CompileOptions::getTemporaryPath(const char* suffix, DynamicString& temporaryPath)
{
	TempAllocator1024 tempAllocator1024;
	DynamicString nameString(tempAllocator1024);
	DynamicString prefix(tempAllocator1024);
	GuidFn::toString(GuidFn::getNewGuid(), prefix);

	dataFileSystem.getAbsolutePath(RIO_TEMP_DIRECTORY, nameString);

	PathFn::join(temporaryPath, nameString.getCStr(), prefix.getCStr());
	temporaryPath += '.';
	temporaryPath += suffix;
}

void CompileOptions::deleteFile(const char* path)
{
	dataFileSystem.deleteFile(path);
}

void CompileOptions::write(const void* data, uint32_t size)
{
	ArrayFn::push(this->outputBuffer, (const char*)data, size);
}

void CompileOptions::write(const Buffer& data)
{
	ArrayFn::push(this->outputBuffer, ArrayFn::begin(data), ArrayFn::getCount(data));
}

const char* CompileOptions::getPlatformName() const
{
	return platformName;
}

const Vector<DynamicString>& CompileOptions::getDependencyList() const
{
	return this->dependencyList;
}

void CompileOptions::addDependency(const char* path)
{
	TempAllocator256 tempAllocator256;
	DynamicString dependency(tempAllocator256);
	dependency += path;
	VectorFn::pushBack(this->dependencyList, dependency);
}

int CompileOptions::runExternalCompiler(const char* const* argumentList, StringStream& outputStringStream)
{
	return OsFn::executeProcess(argumentList, outputStringStream);
}

} // namespace Rio
