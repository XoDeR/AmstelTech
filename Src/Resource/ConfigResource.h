#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

namespace ConfigResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resourceData);

} // namespace ConfigResourceInternalFn

} // namespace Rio
