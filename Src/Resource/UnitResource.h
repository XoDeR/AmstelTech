#pragma once

#include "Core/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

namespace Rio
{

struct UnitResource
{
	uint32_t version = UINT32_MAX;;
	uint32_t unitListCount = 0;
	uint32_t componentTypeListCount = 0;
};

struct ComponentData
{
	StringId32 type;
	uint32_t instanceListCount = 0;
	uint32_t size = 0;
};

namespace UnitResourceInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace UnitResourceInternalFn

} // namespace Rio
