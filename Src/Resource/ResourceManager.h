#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

// Keeps track and manages resources loaded by ResourceLoader
struct ResourceManager
{
	using LoadFunction = void* (*)(File& file, Allocator& a);
	using OnlineFunction = void (*)(StringId64 name, ResourceManager& resourceManager);
	using OfflineFunction = void (*)(StringId64 name, ResourceManager& resourceManager);
	using UnloadFunction = void (*)(Allocator& allocator, void* resourceData);

	struct ResourcePair
	{
		StringId64 type;
		StringId64 name;

		bool operator<(const ResourcePair& resourcePair) const
		{
			return type < resourcePair.type || (type == resourcePair.type && name < resourcePair.name);
		}
	};

	struct ResourceEntry
	{
		uint32_t referencesCount = 0;
		void* data = nullptr;

		bool operator==(const ResourceEntry& resourceEntry)
		{
			return referencesCount == resourceEntry.referencesCount && data == resourceEntry.data;
		}

		static const ResourceEntry NOT_FOUND;
	};

	struct ResourceTypeData
	{
		uint32_t version = UINT32_MAX;
		LoadFunction load = nullptr;
		OnlineFunction online = nullptr;
		OfflineFunction offline = nullptr;
		UnloadFunction unload = nullptr;
	};

	using ResourceTypeDataMap = SortMap<StringId64, ResourceTypeData>;
	using ResourceMap = SortMap<ResourcePair, ResourceEntry>;

	ProxyAllocator resourceProxyAllocator;
	ResourceLoader* resourceLoader = nullptr;
	ResourceTypeDataMap resourceTypeDataMap;
	ResourceMap resourceMap;
	bool autoloadEnabled = false;

	void onResourceOnline(StringId64 type, StringId64 name);
	void onResourceOffline(StringId64 type, StringId64 name);
	void onResourceUnload(StringId64 type, void* resourceData);
	void completeResourceLoadRequest(StringId64 type, StringId64 name, void* resourceData);

	// Uses <resourceLoader> to load resources
	ResourceManager(ResourceLoader& resourceLoader);
	~ResourceManager();

	// Loads the resource (<type>, <name>)
	// You can check whether the resource is available with hasResource()
	void load(StringId64 type, StringId64 name);

	// Unloads the resource (<type>, <name>)
	void unload(StringId64 type, StringId64 name);

	// Reloads the resource (<type>, <name>)
	// The user has to manually update all the references to the old resource
	void reload(StringId64 type, StringId64 name);

	// Returns whether the manager has the resource (<type>, <name>)
	bool hasResource(StringId64 type, StringId64 name);

	// Returns the data of the resource (<type>, <name>)
	const void* getResourceData(StringId64 type, StringId64 name);

	// Sets whether resources should be automatically loaded when accessed
	void enableAutoload(bool autoloadEnabled);

	// Blocks until all load() requests have been completed
	void flush();

	// Completes all load() requests which have been loaded by ResourceLoader
	void completeLoadRequests();

	// Registers a new resource <type> into the resource manager
	void registerNewResourceType(StringId64 type, uint32_t version
		, LoadFunction loadFunction
		, UnloadFunction unloadFunction
		, OnlineFunction onlineFunction
		, OfflineFunction offlineFunction
	);
};

} // namespace Rio
