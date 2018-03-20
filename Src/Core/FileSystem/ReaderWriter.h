#pragma once

#include "Core/FileSystem/File.h"
#include "Core/Types.h"

namespace Rio
{

// A writer that offers a convenient way to write to a File
struct BinaryWriter
{
	File& file;

	BinaryWriter(File& file)
		: file(file)
	{
	}

	void write(const void* data, uint32_t size)
	{
		file.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		file.write(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
	{
		file.skip(bytes);
	}
};

// A reader that offers a convenient way to read from a File
struct BinaryReader
{
	File& file;

	BinaryReader(File& file)
		: file(file)
	{
	}

	void read(void* data, uint32_t size)
	{
		file.read(data, size);
	}

	template <typename T>
	void read(T& data)
	{
		file.read(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
	{
		file.skip(bytes);
	}
};

} // namespace Rio
