#pragma once

#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

// Collection of resources to load in a batch
struct ResourcePackage
{
	uint32_t marker = RESOURCE_PACKAGE_MARKER;

	ResourceManager* resourceManager = nullptr;
	StringId64 packageId;
	const PackageResource* packageResource = nullptr;

	ResourcePackage(StringId64 packageId, ResourceManager& resman);
	~ResourcePackage();

	// Loads all the resources in the package
	// The resources are not immediately available after the call is made, instead, you have to poll for completion with hasLoaded()
	void load();

	// Unloads all the resources in the package
	void unload();

	// Waits until the package has been loaded
	void flush();

	// Returns whether the package has been loaded
	bool hasLoaded() const;
};

} // namespace Rio
