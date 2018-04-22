#pragma once

#include "Core/Containers/Types.h"
#include "Core/Json/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/CompileOptions.h"

namespace Rio
{

struct UnitCompiler
{
	using CompileFunction = Buffer (*)(const char* json, CompileOptions& compileOptions);

	struct ComponentTypeData
	{
		ALLOCATOR_AWARE;

		CompileFunction compileFunction = nullptr;

		uint32_t unitListCount = 0;
		Array<uint32_t> unitIndexList;
		Buffer componentTypeDataBuffer;

		ComponentTypeData(Allocator& a)
			: unitIndexList(a)
			, componentTypeDataBuffer(a)
		{
		}
	};

	struct ComponentTypeInfo
	{
		StringId32 componentType;
		float spawnOrder = 0.0f;

		bool operator<(const ComponentTypeInfo& componentTypeInfo) const
		{
			return spawnOrder < componentTypeInfo.spawnOrder;
		}
	};

	CompileOptions& compileOptions;
	uint32_t unitListCount = 0;
	SortMap<StringId32, ComponentTypeData> componentTypeDataMap;
	Array<ComponentTypeInfo> componentTypeInfoList;

public:

	UnitCompiler(CompileOptions& compileOptions);

private:

	void registerComponentCompileFunction(const char* type, CompileFunction compileFunction, float spawnOrder);
	void registerComponentCompileFunction(StringId32 type, CompileFunction compileFunction, float spawnOrder);
	Buffer compileComponent(StringId32 type, const char* json);
	void addComponentData(StringId32 type, const Buffer& data, uint32_t unit_index);
	Buffer readUnitData(const char* path);

public:

	void compileUnit(const char* path);

private:

	void compileUnitFromJson(const char* json);

public:

	void compileUnitListFromJson(const char* json);
	Buffer getBlob();
};

} // namespace Rio
