#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

struct SpriteResource
{
	uint32_t version = UINT32_MAX;
	Obb obb;
	uint32_t vertexListCount = 0;
};

namespace SpriteResourceInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace SpriteResourceInternalFn

namespace SpriteResourceFn
{
	// Returns the frame data for the frame <frameIndex>
	const float* getFrameData(const SpriteResource* spriteResource, uint32_t frameIndex);

} // namespace SpriteResourceFn

struct SpriteAnimationResource
{
	uint32_t version = UINT32_MAX;
	uint32_t frameListCount = 0;
	float totalTime = 0.0f;
};

namespace SpriteAnimationResourceInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace SpriteAnimationResourceInternalFn

namespace SpriteAnimationResourceFn
{
	// Returns the frameList of the sprite animation <spriteAnimationResource>
	const uint32_t* getSpriteAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource);

} // namespace SpriteAnimationResourceFn

} // namespace Rio
