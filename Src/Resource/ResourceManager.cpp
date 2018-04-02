#include "Resource/ResourceManager.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/SortMap.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Resource/ResourceLoader.h"

namespace Rio
{

const ResourceManager::ResourceEntry ResourceManager::ResourceEntry::NOT_FOUND = 
{ 
	0xffffffffu, 
	nullptr 
};

ResourceManager::ResourceManager(ResourceLoader& resourceLoader)
	: resourceProxyAllocator(getDefaultAllocator(), "resource")
	, resourceLoader(&resourceLoader)
	, resourceTypeDataMap(getDefaultAllocator())
	, resourceMap(getDefaultAllocator())
{
}

ResourceManager::~ResourceManager()
{
	auto current = SortMapFn::begin(resourceMap);
	auto end = SortMapFn::end(resourceMap);
	for (; current != end; ++current)
	{
		const StringId64 type = current->first.type;
		const StringId64 name = current->first.name;
		onResourceOffline(type, name);
		onResourceUnload(type, current->second.data);
	}
}

void ResourceManager::load(StringId64 type, StringId64 name)
{
	ResourcePair resourcePair =
	{ 
		type, 
		name 
	};

	ResourceEntry& resourceEntry = SortMapFn::get(resourceMap, resourcePair, ResourceEntry::NOT_FOUND);

	if (resourceEntry == ResourceEntry::NOT_FOUND)
	{
		StringId64 mix;
		mix.id = type.id ^ name.id;

		TempAllocator256 tempAllocator256;
		DynamicString path(tempAllocator256);
		mix.toString(path);

		RIO_ASSERT(this->resourceLoader->canLoad(type, name), "Can't load resource #ID(%s)", path.getCStr());

		ResourceTypeData resourceTypeData;
		resourceTypeData.version = UINT32_MAX;
		resourceTypeData.load = nullptr;
		resourceTypeData.online = nullptr;
		resourceTypeData.offline = nullptr;
		resourceTypeData.unload = nullptr;
		resourceTypeData = SortMapFn::get(this->resourceTypeDataMap, type, resourceTypeData);

		ResourceRequest resourceRequest;
		resourceRequest.type = type;
		resourceRequest.name = name;
		resourceRequest.version = resourceTypeData.version;
		resourceRequest.loadFunction = resourceTypeData.load;
		resourceRequest.allocator = &resourceProxyAllocator;
		resourceRequest.data = nullptr;

		this->resourceLoader->addLoadResourceRequest(resourceRequest);

		return;
	}

	resourceEntry.referencesCount++;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	flush();

	ResourcePair resourcePair =
	{ 
		type, 
		name 
	};

	ResourceEntry& resourceEntry = SortMapFn::get(resourceMap, resourcePair, ResourceEntry::NOT_FOUND);

	if (--resourceEntry.referencesCount == 0)
	{
		onResourceOffline(type, name);
		onResourceUnload(type, resourceEntry.data);

		SortMapFn::remove(resourceMap, resourcePair);
		SortMapFn::sort(resourceMap);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair resourcePair =
	{ 
		type, 
		name 
	};

	const ResourceEntry& resourceEntry = SortMapFn::get(resourceMap, resourcePair, ResourceEntry::NOT_FOUND);
	const uint32_t oldReferencesCount = resourceEntry.referencesCount;

	unload(type, name);
	load(type, name);
	flush();

	ResourceEntry& newResourceEntry = SortMapFn::get(resourceMap, resourcePair, ResourceEntry::NOT_FOUND);
	newResourceEntry.referencesCount = oldReferencesCount;
}

bool ResourceManager::hasResource(StringId64 type, StringId64 name)
{
	const ResourcePair resourcePair = 
	{ 
		type, 
		name 
	};

	return this->autoloadEnabled ? true : SortMapFn::has(this->resourceMap, resourcePair);
}

const void* ResourceManager::getResourceData(StringId64 type, StringId64 name)
{
	const ResourcePair resourcePair =
	{ 
		type, 
		name 
	};

	StringId64 mix;
	mix.id = type.id ^ name.id;

	TempAllocator256 tempAllocator256;
	DynamicString path(tempAllocator256);
	mix.toString(path);

	RIO_ASSERT(hasResource(type, name), "Resource not loaded #ID(%s)", path.getCStr());

	if (autoloadEnabled && !SortMapFn::has(resourceMap, resourcePair))
	{
		load(type, name);
		flush();
	}

	const ResourceEntry& resourceEntry = SortMapFn::get(resourceMap, resourcePair, ResourceEntry::NOT_FOUND);
	return resourceEntry.data;
}

void ResourceManager::enableAutoload(bool autoloadEnabled)
{
	this->autoloadEnabled = autoloadEnabled;
}

void ResourceManager::flush()
{
	this->resourceLoader->flush();
	completeLoadRequests();
}

void ResourceManager::completeLoadRequests()
{
	TempAllocator1024 tempAllocator1024;
	Array<ResourceRequest> loadedResourceRequestList(tempAllocator1024);
	this->resourceLoader->getLoaded(loadedResourceRequestList);

	for (uint32_t i = 0; i < ArrayFn::getCount(loadedResourceRequestList); ++i)
	{
		completeResourceLoadRequest(loadedResourceRequestList[i].type, loadedResourceRequestList[i].name, loadedResourceRequestList[i].data);
	}
}

void ResourceManager::completeResourceLoadRequest(StringId64 type, StringId64 name, void* resourceData)
{
	ResourceEntry resourceEntry;
	resourceEntry.referencesCount = 1;
	resourceEntry.data = resourceData;

	ResourcePair resourcePair =
	{ 
		type, 
		name 
	};

	SortMapFn::set(resourceMap, resourcePair, resourceEntry);
	SortMapFn::sort(resourceMap);

	onResourceOnline(type, name);
}

void ResourceManager::registerNewResourceType(StringId64 type, uint32_t version
	, LoadFunction loadFunction
	, UnloadFunction unloadFunction
	, OnlineFunction onlineFunction
	, OfflineFunction offlineFunction
)
{
	ResourceTypeData resourceTypeData;
	resourceTypeData.version = version;
	resourceTypeData.load = loadFunction;
	resourceTypeData.online = onlineFunction;
	resourceTypeData.offline = offlineFunction;
	resourceTypeData.unload = unloadFunction;

	SortMapFn::set(this->resourceTypeDataMap, type, resourceTypeData);
	SortMapFn::sort(this->resourceTypeDataMap);
}

void ResourceManager::onResourceOnline(StringId64 type, StringId64 name)
{
	OnlineFunction onlineFunction = SortMapFn::get(this->resourceTypeDataMap, type, ResourceTypeData()).online;

	if (onlineFunction != nullptr)
	{
		onlineFunction(name, *this);
	}
}

void ResourceManager::onResourceOffline(StringId64 type, StringId64 name)
{
	OfflineFunction offlineFunction = SortMapFn::get(this->resourceTypeDataMap, type, ResourceTypeData()).offline;

	if (offlineFunction != nullptr)
	{
		offlineFunction(name, *this);
	}
}

void ResourceManager::onResourceUnload(StringId64 type, void* data)
{
	UnloadFunction unloadFunction = SortMapFn::get(this->resourceTypeDataMap, type, ResourceTypeData()).unload;

	if (unloadFunction != nullptr)
	{
		unloadFunction(resourceProxyAllocator, data);
	}
	else
	{
		resourceProxyAllocator.deallocate(data);
	}
}

} // namespace Rio
