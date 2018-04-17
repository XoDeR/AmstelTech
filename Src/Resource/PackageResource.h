#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Strings/Types.h"

#include "Resource/Types.h"

namespace Rio
{

struct PackageResource
{
	struct Resource
	{
		StringId64 type;
		StringId64 name;

		Resource(StringId64 type, StringId64 name)
			: type(type)
			, name(name)
		{
		}

		bool operator<(const Resource& resource) const
		{
			return type < resource.type;
		}

		bool operator==(const Resource& resource) const
		{
			return type == resource.type && name == resource.name;
		}
	};

	PackageResource(Allocator& a)
		: resourceList(a)
	{
	}

	Array<Resource> resourceList;
};

namespace PackageResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resourceData);

} // namespace PackageResourceInternalFn

} // namespace Rio
