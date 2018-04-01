#include "Resource/FontResource.h"

#include "Core/Containers/Array.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/String.h"

#include "Resource/CompileOptions.h"
#include "Resource/Types.h"

#include <algorithm> // std::sort

namespace Rio
{

namespace FontResourceInternalFn
{
	struct GlyphInfo
	{
		CodePoint codePoint = UINT32_MAX;
		GlyphData glyphData;

		bool operator<(const GlyphInfo& glyphInfo) const
		{
			return codePoint < glyphInfo.codePoint;
		}
	};

	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject object(tempAllocator4096);
		JsonArray glyphListJson(tempAllocator4096);

		RJsonFn::parse(buffer, object);
		RJsonFn::parseArray(object["glyphList"], glyphListJson);

		const uint32_t textureSize = RJsonFn::parseInt32(object["size"]);
		const uint32_t fontSize = RJsonFn::parseInt32(object["fontSize"]);
		const uint32_t glyphsCount = ArrayFn::getCount(glyphListJson);

		DATA_COMPILER_ASSERT(fontSize > 0
			, compileOptions
			, "Font size must be > 0"
			);

		Array<GlyphInfo> glyphInfoList(getDefaultAllocator());
		ArrayFn::resize(glyphInfoList, glyphsCount);
		for (uint32_t i = 0; i < glyphsCount; ++i)
		{
			TempAllocator512 tempAllocator512;
			JsonObject jsonObject(tempAllocator512);
			RJsonFn::parse(glyphListJson[i], jsonObject);

			glyphInfoList[i].codePoint = RJsonFn::parseInt32(jsonObject["id"]);
			glyphInfoList[i].glyphData.x = RJsonFn::parseFloat(jsonObject["x"]);
			glyphInfoList[i].glyphData.y = RJsonFn::parseFloat(jsonObject["y"]);
			glyphInfoList[i].glyphData.width = RJsonFn::parseFloat(jsonObject["width"]);
			glyphInfoList[i].glyphData.height = RJsonFn::parseFloat(jsonObject["height"]);
			glyphInfoList[i].glyphData.xOffset = RJsonFn::parseFloat(jsonObject["xOffset"]);
			glyphInfoList[i].glyphData.yOffset = RJsonFn::parseFloat(jsonObject["yOffset"]);
			glyphInfoList[i].glyphData.xAdvance = RJsonFn::parseFloat(jsonObject["xAdvance"]);
		}

		std::sort(ArrayFn::begin(glyphInfoList), ArrayFn::end(glyphInfoList));

		compileOptions.write(RESOURCE_VERSION_FONT);
		compileOptions.write(textureSize);
		compileOptions.write(fontSize);
		compileOptions.write(glyphsCount);

		for (uint32_t i = 0; i < glyphsCount; ++i)
		{
			compileOptions.write(glyphInfoList[i].codePoint);
		}

		for (uint32_t i = 0; i < glyphsCount; ++i)
		{
			compileOptions.write(glyphInfoList[i].glyphData.x);
			compileOptions.write(glyphInfoList[i].glyphData.y);
			compileOptions.write(glyphInfoList[i].glyphData.width);
			compileOptions.write(glyphInfoList[i].glyphData.height);
			compileOptions.write(glyphInfoList[i].glyphData.xOffset);
			compileOptions.write(glyphInfoList[i].glyphData.yOffset);
			compileOptions.write(glyphInfoList[i].glyphData.xAdvance);
		}
	}

} // namespace FontResourceInternalFn

namespace FontResourceFn
{
	const GlyphData* getGlyphData(const FontResource* fontResource, CodePoint codePoint)
	{
		RIO_ASSERT(codePoint < fontResource->glyphsCount, "Index out of bounds");

		const CodePoint* codePointList = (CodePoint*)&fontResource[1];
		const GlyphData* glyphDataList = (GlyphData*)(codePointList + fontResource->glyphsCount);

		for (uint32_t i = 0; i < fontResource->glyphsCount; ++i)
		{
			if (codePointList[i] == codePoint)
			{
				return &glyphDataList[i];
			}
		}

		RIO_FATAL("Glyph not found");

		return nullptr;
	}

} // namespace FontResourceFn

} // namespace Rio
