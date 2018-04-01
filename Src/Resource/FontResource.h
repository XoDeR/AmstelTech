#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

struct FontResource
{
	uint32_t version = UINT32_MAX;
	uint32_t textureSize = UINT32_MAX;
	uint32_t fontSize = UINT32_MAX;
	uint32_t glyphsCount = UINT32_MAX;
};

struct GlyphData
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	float xAdvance = 0.0f;
};

using CodePoint = uint32_t;

namespace FontResourceInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace FontResourceInternalFn

namespace FontResourceFn
{
	// Returns the glyph for the code point <codePoint>
	const GlyphData* getGlyphData(const FontResource* fontResource, CodePoint codePoint);

} // namespace FontResourceFn

} // namespace Rio
