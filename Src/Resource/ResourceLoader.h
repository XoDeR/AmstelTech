#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Types.h"

namespace Rio
{

struct ResourceRequest
{
	using LoadFunction = void* (*)(File& file, Allocator& a);

	StringId64 type;
	StringId64 name;
	uint32_t version = UINT32_MAX;
	LoadFunction loadFunction = nullptr;
	Allocator* allocator = nullptr;
	void* data = nullptr;
};

// Loads resources in a background thread
struct ResourceLoader
{
	FileSystem& dataFileSystem;

	Queue<ResourceRequest> resourceRequestQueue;
	Queue<ResourceRequest> resourceRequestLoadedQueue;

	Thread thread;
	Mutex mutex;
	Mutex mutexLoaded;

	bool exit = false;

	uint32_t getRequestsCount();
	void addLoaded(ResourceRequest rr);

	// Do not call explicitly
	int32_t run();

	// Read resources from <dataFileSystem>
	ResourceLoader(FileSystem& dataFileSystem);
	~ResourceLoader();

	// Returns whether the resource (type, name) can be loaded
	bool canLoad(StringId64 type, StringId64 name);

	// Adds a request for loading the resource described by <resourceRequest>
	void addLoadResourceRequest(const ResourceRequest& resourceRequest);

	// Blocks until all pending requests have been processed
	void flush();

	// Returns all the resources that have been loaded
	void getLoaded(Array<ResourceRequest>& loaded);
};

} // namespace Rio
