#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/Types.h"

#include "Resource/Types.h"

#include <stdarg.h>

#define DATA_COMPILER_ASSERT(condition, compileOptions, message, ...)	\
	do																	\
	{																	\
		if (!(condition))												\
			compileOptions.error(message, ## __VA_ARGS__);				\
	} while(0)

#define DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name, compileOptions)	\
	DATA_COMPILER_ASSERT(compileOptions.doesResourceExist(type, name)		\
		, compileOptions													\
		, "Resource does not exist: '%s.%s'"								\
		, name																\
		, type																\
		)

#define DATA_COMPILER_ASSERT_FILE_EXISTS(name, compileOptions)	\
	DATA_COMPILER_ASSERT(compileOptions.doesFileExist(name)		\
		, compileOptions										\
		, "File does not exist: '%s'"							\
		, name													\
		)

namespace Rio
{

struct CompileOptions
{
	DataCompiler& dataCompiler;
	FileSystem& dataFileSystem;
	DynamicString sourcePath;
	Buffer& outputBuffer;
	const char* platformName = nullptr;
	Vector<DynamicString> dependencyList;

	CompileOptions(DataCompiler& dataCompiler, FileSystem& dataFileSystem, DynamicString& sourcePath, Buffer& outputBuffer, const char* platformName);
	
	void error(const char* message, va_list argumentList);
	void error(const char* message, ...);

	const char* getSourcePath();
	bool doesFileExist(const char* path);
	bool doesResourceExist(const char* type, const char* name);

	Buffer readTemporary(const char* path);
	void writeTemporary(const char* path, const char* data, uint32_t size);
	void writeTemporary(const char* path, const Buffer& data);

	Buffer read();
	Buffer read(const char* path);
	void getAbsolutePath(const char* path, DynamicString& absolutePath);
	void getTemporaryPath(const char* suffix, DynamicString& temporaryPath);
	void deleteFile(const char* path);
	void write(const void* data, uint32_t size);

	template <typename T>
	void write(const T& data)
	{
		write(&data, sizeof(data));
	}

	void write(const Buffer& data);
	const char* getPlatformName() const;
	const Vector<DynamicString>& getDependencyList() const;
	void addDependency(const char* path);
	int runExternalCompiler(const char* const* argumentList, StringStream& outputStringStream);
};

} // namespace Rio
