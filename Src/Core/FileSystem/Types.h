#pragma once

namespace Rio
{

struct File;
struct FileMonitor;
struct Filesystem;

// Enumerates file open modes
struct FileOpenMode
{
	enum Enum
	{
		READ,
		WRITE
	};
};

} // namespace Rio
