#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

#include "World/Material.h"

namespace Rio
{

struct MaterialManager
{
	Allocator* allocator = nullptr;
	ResourceManager* resourceManager = nullptr;
	SortMap<StringId64, Material*> materialNameMap;

	MaterialManager(Allocator& a, ResourceManager& resourceManager);
	~MaterialManager();

	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resourceData);

	// Creates the material <id>
	void createMaterial(StringId64 id);

	// Destroys the material <id>
	void destroyMaterial(StringId64 id);

	// Returns the material <id>
	Material* getMaterialById(StringId64 id);
};

} // namespace Rio
