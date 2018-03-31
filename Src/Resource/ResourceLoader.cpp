#include "Resource/ResourceLoader.h"

#include "Config.h"

#include "Core/Containers/Queue.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/Path.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Strings/DynamicString.h"

namespace Rio
{

static int32_t threadProcedure(void* thiz)
{
	return ((ResourceLoader*)thiz)->run();
}

ResourceLoader::ResourceLoader(FileSystem& dataFileSystem)
	: dataFileSystem(dataFileSystem)
	, resourceRequestQueue(getDefaultAllocator())
	, resourceRequestLoadedQueue(getDefaultAllocator())
{
	thread.start(threadProcedure, this);
}

ResourceLoader::~ResourceLoader()
{
	exit = true;
	thread.stop();
}

bool ResourceLoader::canLoad(StringId64 type, StringId64 name)
{
	StringId64 mix;
	mix.id = type.id ^ name.id;

	TempAllocator128 ta;
	DynamicString resourcePath(ta);
	mix.toString(resourcePath);

	DynamicString path(ta);
	PathFn::join(path, RIO_DATA_DIRECTORY, resourcePath.getCStr());

	return dataFileSystem.exists(path.getCStr());
}

void ResourceLoader::addLoadResourceRequest(const ResourceRequest& resourceRequest)
{
	ScopedMutex scopedMutex(mutex);
	QueueFn::pushBack(resourceRequestQueue, resourceRequest);
}

void ResourceLoader::flush()
{
	while (getRequestsCount() != 0)
	{
	}
}

uint32_t ResourceLoader::getRequestsCount()
{
	ScopedMutex scopedMutex(mutex);
	return QueueFn::getCount(resourceRequestQueue);
}

void ResourceLoader::addLoaded(ResourceRequest resourceRequest)
{
	ScopedMutex scopedMutex(mutexLoaded);
	QueueFn::pushBack(resourceRequestLoadedQueue, resourceRequest);
}

void ResourceLoader::getLoaded(Array<ResourceRequest>& loadedResourceRequest)
{
	ScopedMutex scopedMutex(mutexLoaded);

	const uint32_t loadedCount = QueueFn::getCount(resourceRequestLoadedQueue);
	ArrayFn::reserve(loadedResourceRequest, loadedCount);

	for (uint32_t i = 0; i < loadedCount; ++i)
	{
		ArrayFn::pushBack(loadedResourceRequest, QueueFn::getFront(resourceRequestLoadedQueue));
		QueueFn::popFront(resourceRequestLoadedQueue);
	}
}

int32_t ResourceLoader::run()
{
	while (exit == false)
	{
		mutex.lock();
		if (QueueFn::getIsEmpty(resourceRequestQueue))
		{
			mutex.unlock();
			OsFn::sleep(16);
			continue;
		}

		ResourceRequest resourceRequest = QueueFn::getFront(resourceRequestQueue);
		mutex.unlock();

		StringId64 mix;
		mix.id = resourceRequest.type.id ^ resourceRequest.name.id;

		TempAllocator128 ta;
		DynamicString resourcePath(ta);
		mix.toString(resourcePath);

		DynamicString path(ta);
		PathFn::join(path, RIO_DATA_DIRECTORY, resourcePath.getCStr());

		if (dataFileSystem.exists(path.getCStr()))
		{
			File* file = dataFileSystem.open(path.getCStr(), FileOpenMode::READ);

			if (resourceRequest.loadFunction)
			{
				resourceRequest.data = resourceRequest.loadFunction(*file, *resourceRequest.allocator);
			}
			else
			{
				const uint32_t size = file->getFileSize();
				void* data = resourceRequest.allocator->allocate(size);
				file->read(data, size);
				RIO_ASSERT(*(uint32_t*)data == resourceRequest.version, "Error: Wrong resource version");
				resourceRequest.data = data;
			}

			dataFileSystem.close(*file);

			addLoaded(resourceRequest);
		}
		else
		{
			RIO_FATAL("No file path.getCStr() present");
		}
		
		mutex.lock();

		QueueFn::popFront(resourceRequestQueue);
		
		mutex.unlock();
	}

	return 0;
}

} // namespace Rio
