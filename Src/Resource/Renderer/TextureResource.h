#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/Types.h"

#include "Resource/Types.h"
#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct TextureResource
{
	const RioRenderer::Memory* memory = nullptr;
	RioRenderer::TextureHandle handle;
};

namespace TextureResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void online(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resourceData);

} // namespace TextureResourceInternalFn

} // namespace Rio
