#include "Resource/UnitCompiler.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/Containers/SortMap.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Math/Math.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Resource/CompileOptions.h"

#if AMSTEL_ENGINE_PHYSICS
#include "Resource/Physics/PhysicsResource.h"
#endif // AMSTEL_ENGINE_PHYSICS

#include "Resource/UnitResource.h"

#include "World/Types.h"

namespace Rio
{

struct ProjectionInfo
{
	const char* name = nullptr;
	ProjectionType::Enum type = ProjectionType::COUNT;
};

static const ProjectionInfo projectionInfoMapStatic[] =
{
	{ "perspective", ProjectionType::PERSPECTIVE },
	{ "orthographic", ProjectionType::ORTHOGRAPHIC }
};
RIO_STATIC_ASSERT(countof(projectionInfoMapStatic) == ProjectionType::COUNT);

struct LightInfo
{
	const char* name = nullptr;
	LightType::Enum type = LightType::COUNT;
};

static const LightInfo lightInfoMapStatic[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni", LightType::OMNI },
	{ "spot", LightType::SPOT }
};
RIO_STATIC_ASSERT(countof(lightInfoMapStatic) == LightType::COUNT);

static ProjectionType::Enum getProjectionTypeByName(const char* name)
{
	for (uint32_t i = 0; i < countof(projectionInfoMapStatic); ++i)
	{
		if (strcmp(name, projectionInfoMapStatic[i].name) == 0)
		{
			return projectionInfoMapStatic[i].type;
		}
	}

	return ProjectionType::COUNT;
}

static LightType::Enum getLightTypeByName(const char* name)
{
	for (uint32_t i = 0; i < countof(lightInfoMapStatic); ++i)
	{
		if (strcmp(name, lightInfoMapStatic[i].name) == 0)
		{
			return lightInfoMapStatic[i].type;
		}
	}

	return LightType::COUNT;
}

static Buffer compileTransform(const char* json, CompileOptions& /*compileOptions*/)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject transformJsonObject(tempAllocator4096);
	RJsonFn::parse(json, transformJsonObject);

	TransformDesc transformDesc;
	transformDesc.position = RJsonFn::parseVector3(transformJsonObject["position"]);
	transformDesc.rotation = RJsonFn::parseQuaternion(transformJsonObject["rotation"]);
	transformDesc.scale = RJsonFn::parseVector3(transformJsonObject["scale"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&transformDesc, sizeof(transformDesc));
	return buffer;
}

static Buffer compileCamera(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject cameraJsonObject(tempAllocator4096);
	RJsonFn::parse(json, cameraJsonObject);

	DynamicString projectionTypeString(tempAllocator4096);
	RJsonFn::parseString(cameraJsonObject["projection"], projectionTypeString);

	ProjectionType::Enum projectionType = getProjectionTypeByName(projectionTypeString.getCStr());
	DATA_COMPILER_ASSERT(projectionType != ProjectionType::COUNT
		, compileOptions
		, "Unknown projection type: '%s'"
		, projectionTypeString.getCStr()
		);

	CameraDesc cameraDesc;
	cameraDesc.type = projectionType;
	cameraDesc.fov = RJsonFn::parseFloat(cameraJsonObject["fov"]);
	cameraDesc.nearRange = RJsonFn::parseFloat(cameraJsonObject["nearRange"]);
	cameraDesc.farRange = RJsonFn::parseFloat(cameraJsonObject["farRange"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&cameraDesc, sizeof(cameraDesc));
	return buffer;
}

static Buffer compileMeshRenderer(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject meshJsonObject(tempAllocator4096);
	RJsonFn::parse(json, meshJsonObject);

	DynamicString meshResourceString(tempAllocator4096);
	RJsonFn::parseString(meshJsonObject["meshResource"], meshResourceString);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("mesh"
		, meshResourceString.getCStr()
		, compileOptions
		);

	MeshRendererDesc meshRendererDesc;
	meshRendererDesc.meshResource = RJsonFn::parseResourceId(meshJsonObject["meshResource"]);
	meshRendererDesc.geometryName = RJsonFn::parseStringId(meshJsonObject["geometryName"]);
	meshRendererDesc.materialResource = RJsonFn::parseResourceId(meshJsonObject["material"]);
	meshRendererDesc.visible = RJsonFn::parseBool(meshJsonObject["visible"]);
	meshRendererDesc.padding00[0] = 0;
	meshRendererDesc.padding00[1] = 0;
	meshRendererDesc.padding00[2] = 0;

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&meshRendererDesc, sizeof(meshRendererDesc));
	return buffer;
}

static Buffer compileSpriteRenderer(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject spriteJsonObject(tempAllocator4096);
	RJsonFn::parse(json, spriteJsonObject);

	DynamicString spriteResourceString(tempAllocator4096);
	RJsonFn::parseString(spriteJsonObject["spriteResource"], spriteResourceString);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("sprite"
		, spriteResourceString.getCStr()
		, compileOptions
		);

	SpriteRendererDesc spriteRendererDesc;
	spriteRendererDesc.spriteResource = RJsonFn::parseResourceId(spriteJsonObject["spriteResource"]);
	spriteRendererDesc.materialResource = RJsonFn::parseResourceId(spriteJsonObject["material"]);
	spriteRendererDesc.layer = RJsonFn::parseInt32(spriteJsonObject["layer"]);
	spriteRendererDesc.depth = RJsonFn::parseInt32(spriteJsonObject["depth"]);
	spriteRendererDesc.visible = RJsonFn::parseBool(spriteJsonObject["visible"]);
	spriteRendererDesc.padding00[0] = 0;
	spriteRendererDesc.padding00[1] = 0;
	spriteRendererDesc.padding00[2] = 0;
	spriteRendererDesc.padding01[0] = 0;
	spriteRendererDesc.padding01[1] = 0;
	spriteRendererDesc.padding01[2] = 0;
	spriteRendererDesc.padding01[3] = 0;

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&spriteRendererDesc, sizeof(spriteRendererDesc));
	return buffer;
}

static Buffer compileLight(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject lightJsonObject(tempAllocator4096);
	RJsonFn::parse(json, lightJsonObject);

	DynamicString typeString(tempAllocator4096);
	RJsonFn::parseString(lightJsonObject["type"], typeString);

	LightType::Enum lightType = getLightTypeByName(typeString.getCStr());
	DATA_COMPILER_ASSERT(lightType != LightType::COUNT
		, compileOptions
		, "Unknown light type: '%s'"
		, typeString.getCStr()
		);

	LightDesc lightDesc;
	lightDesc.type = lightType;
	lightDesc.range = RJsonFn::parseFloat(lightJsonObject["range"]);
	lightDesc.intensity = RJsonFn::parseFloat(lightJsonObject["intensity"]);
	lightDesc.spotAngle = RJsonFn::parseFloat(lightJsonObject["spotAngle"]);
	lightDesc.color = RJsonFn::parseVector3(lightJsonObject["color"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&lightDesc, sizeof(lightDesc));
	return buffer;
}

#if AMSTEL_ENGINE_SCRIPT_LUA
static Buffer compileScript(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject scriptJsonObject(tempAllocator4096);
	RJsonFn::parse(json, scriptJsonObject);

	DynamicString scriptResourceString(tempAllocator4096);
	RJsonFn::parseString(scriptJsonObject["scriptResource"], scriptResourceString);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("lua"
		, scriptResourceString.getCStr()
		, compileOptions
		);

	ScriptDesc scriptDesc;
	scriptDesc.scriptResourceName = RJsonFn::parseResourceId(obj["scriptResource"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&scriptDesc, sizeof(scriptDesc));
	return buffer;
}
#endif // AMSTEL_ENGINE_SCRIPT_LUA

static Buffer compileAnimationStateMachine(const char* json, CompileOptions& compileOptions)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject animationStateMachineJsonObject(tempAllocator4096);
	RJsonFn::parse(json, animationStateMachineJsonObject);

	DynamicString stateMachineResourceString(tempAllocator4096);
	RJsonFn::parseString(animationStateMachineJsonObject["stateMachineResource"], stateMachineResourceString);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("stateMachine"
		, stateMachineResourceString.getCStr()
		, compileOptions
		);

	AnimationStateMachineDesc animationStateMachineDesc;
	animationStateMachineDesc.stateMachineResource = RJsonFn::parseResourceId(animationStateMachineJsonObject["stateMachineResource"]);

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&animationStateMachineDesc, sizeof(animationStateMachineDesc));
	return buffer;
}

UnitCompiler::UnitCompiler(CompileOptions& compileOptions)
	: compileOptions(compileOptions)
	, componentTypeDataMap(getDefaultAllocator())
	, componentTypeInfoList(getDefaultAllocator())
{
	registerComponentCompileFunction("transform", &compileTransform, 0.0f);
	registerComponentCompileFunction("camera", &compileCamera, 1.0f);
	registerComponentCompileFunction("meshRenderer", &compileMeshRenderer, 1.0f);
	registerComponentCompileFunction("spriteRenderer", &compileSpriteRenderer, 1.0f);
	registerComponentCompileFunction("light", &compileLight, 1.0f);
	registerComponentCompileFunction("animationStateMachine", &compileAnimationStateMachine, 1.0f);

#if AMSTEL_ENGINE_SCRIPT_LUA
	registerComponentCompileFunction("script", &compileScript, 1.0f);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_PHYSICS
	registerComponentCompileFunction("collider", &PhysicsResourceInternalFn::compileCollider, 1.0f);
	registerComponentCompileFunction("actor", &PhysicsResourceInternalFn::compileActor, 2.0f);
	registerComponentCompileFunction("joint", &PhysicsResourceInternalFn::compileJoint, 3.0f);
#endif // AMSTEL_ENGINE_PHYSICS
}

Buffer UnitCompiler::readUnitData(const char* path)
{
	Buffer buffer = this->compileOptions.read(path);
	ArrayFn::pushBack(buffer, '\0');
	return buffer;
}

void UnitCompiler::compileUnit(const char* path)
{
	compileUnitFromJson(ArrayFn::begin(readUnitData(path)));
}

void UnitCompiler::compileUnitFromJson(const char* json)
{
	Buffer data(getDefaultAllocator());
	ArrayFn::reserve(data, 1024*1024);

	uint32_t prefabListCount = 1;

	TempAllocator4096 tempAllocator4096;
	JsonObject prefabList[4] = 
	{ 
		JsonObject(tempAllocator4096), 
		JsonObject(tempAllocator4096), 
		JsonObject(tempAllocator4096), 
		JsonObject(tempAllocator4096) 
	};

	RJsonFn::parse(json, prefabList[0]);

	for (uint32_t i = 0; i < countof(prefabList); ++i, ++prefabListCount)
	{
		const JsonObject& prefabJsonObject = prefabList[i];

		if (!JsonObjectFn::has(prefabJsonObject, "prefab"))
		{
			break;
		}

		TempAllocator512 tempAllocator512;
		DynamicString path(tempAllocator512);
		RJsonFn::parseString(prefabJsonObject["prefab"], path);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("unit"
			, path.getCStr()
			, this->compileOptions
			);
		path += ".unit";

		Buffer buffer = readUnitData(path.getCStr());
		const char* dataEnd = ArrayFn::end(data);
		ArrayFn::push(data, ArrayFn::begin(buffer), ArrayFn::getCount(buffer));
		RJsonFn::parse(dataEnd, prefabList[i + 1]);
	}

	JsonObject& prefabRootJsonObject = prefabList[prefabListCount - 1];
	JsonObject prefabRootComponentListJsonObject(tempAllocator4096);
	RJsonFn::parse(prefabRootJsonObject["componentList"], prefabRootComponentListJsonObject);

	if (prefabListCount > 1)
	{
		// Merge prefabs' components
		for (uint32_t i = 0; i < prefabListCount; ++i)
		{
			const JsonObject& prefabJsonObject = prefabList[prefabListCount - i - 1];

			if (!JsonObjectFn::has(prefabJsonObject, "modifiedComponentList"))
			{
				continue;
			}

			JsonObject modifiedComponentListJsonObject(tempAllocator4096);
			RJsonFn::parse(prefabJsonObject["modifiedComponentList"], modifiedComponentListJsonObject);

			auto currentModifiedComponentPair = JsonObjectFn::begin(modifiedComponentListJsonObject);
			auto endModifiedComponentPair = JsonObjectFn::end(modifiedComponentListJsonObject);
			for (; currentModifiedComponentPair != endModifiedComponentPair; ++currentModifiedComponentPair)
			{
				const FixedString key = currentModifiedComponentPair->pair.first;
				const FixedString id(&key.getCStr()[1], key.getLength()-1);
				const char* valueString = currentModifiedComponentPair->pair.second;

				// TODO make better version
				MapFn::remove(prefabRootComponentListJsonObject.jsonMap, id);
				MapFn::set(prefabRootComponentListJsonObject.jsonMap, id, valueString);
			}
		}
	}

	if (JsonObjectFn::getSize(prefabRootComponentListJsonObject) > 0)
	{
		auto currentPrefabRootComponentPair = JsonObjectFn::begin(prefabRootComponentListJsonObject);
		auto endPrefabRootComponentPair = JsonObjectFn::end(prefabRootComponentListJsonObject);
		for (; currentPrefabRootComponentPair != endPrefabRootComponentPair; ++currentPrefabRootComponentPair)
		{
			const char* valueString = currentPrefabRootComponentPair->pair.second;

			TempAllocator512 tempAllocator512;
			JsonObject componentJsonObject(tempAllocator512);
			RJsonFn::parse(valueString, componentJsonObject);

			const StringId32 type = RJsonFn::parseStringId(componentJsonObject["type"]);

			Buffer buffer = compileComponent(type, componentJsonObject["data"]);
			addComponentData(type, buffer, this->unitListCount);
		}
	}

	++this->unitListCount;
}

void UnitCompiler::compileUnitListFromJson(const char* json)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject unitListJsonObject(tempAllocator4096);
	RJsonFn::parse(json, unitListJsonObject);

	auto currentUnitPair = JsonObjectFn::begin(unitListJsonObject);
	auto endUnitPair = JsonObjectFn::end(unitListJsonObject);
	for (; currentUnitPair != endUnitPair; ++currentUnitPair)
	{
		compileUnitFromJson(currentUnitPair->pair.second);
	}
}

Buffer UnitCompiler::getBlob()
{
	UnitResource unitResource;
	unitResource.version = RESOURCE_VERSION_UNIT;
	unitResource.unitListCount = this->unitListCount;
	unitResource.componentTypeListCount = 0;

	auto currentComponentTypeDataPair = SortMapFn::begin(componentTypeDataMap);
	auto endComponentTypeDataPair = SortMapFn::end(componentTypeDataMap);
	for (; currentComponentTypeDataPair != endComponentTypeDataPair; ++currentComponentTypeDataPair)
	{
		const uint32_t unitListCount = currentComponentTypeDataPair->second.unitListCount;

		if (unitListCount > 0)
		{
			++unitResource.componentTypeListCount;
		}
	}

	Buffer buffer(getDefaultAllocator());
	ArrayFn::push(buffer, (char*)&unitResource, sizeof(unitResource));

	for (uint32_t i = 0; i < ArrayFn::getCount(this->componentTypeInfoList); ++i)
	{
		const StringId32 type = this->componentTypeInfoList[i].componentType;
		const ComponentTypeData& componentTypeData = SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator()));

		const Buffer& data = componentTypeData.componentTypeDataBuffer;
		const Array<uint32_t>& unitIndexList = componentTypeData.unitIndexList;
		const uint32_t unitCount = componentTypeData.unitListCount;

		if (unitCount > 0)
		{
			ComponentData componentData;
			componentData.type = type;
			componentData.instanceListCount = unitCount;
			componentData.size = ArrayFn::getCount(data) + sizeof(uint32_t)*ArrayFn::getCount(unitIndexList);

			const uint32_t padding = componentData.size % alignof(componentData);
			componentData.size += padding;

			ArrayFn::push(buffer, (char*)&componentData, sizeof(componentData));
			ArrayFn::push(buffer, (char*)ArrayFn::begin(unitIndexList), sizeof(uint32_t)*ArrayFn::getCount(unitIndexList));
			ArrayFn::push(buffer, ArrayFn::begin(data), ArrayFn::getCount(data));

			// Proper padding
			for (uint32_t i = 0; i < padding; ++i)
			{
				ArrayFn::pushBack(buffer, (char)0);
			}
		}
	}

	return buffer;
}

void UnitCompiler::addComponentData(StringId32 type, const Buffer& data, uint32_t unitIndex)
{
	ComponentTypeData& componentTypeData = const_cast<ComponentTypeData&>(SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator())));

	ArrayFn::push(componentTypeData.componentTypeDataBuffer, ArrayFn::begin(data), ArrayFn::getCount(data));
	ArrayFn::pushBack(componentTypeData.unitIndexList, unitIndex);
	++componentTypeData.unitListCount;
}

void UnitCompiler::registerComponentCompileFunction(const char* type, CompileFunction compileFunction, float spawnOrder)
{
	registerComponentCompileFunction(StringId32(type), compileFunction, spawnOrder);
}

void UnitCompiler::registerComponentCompileFunction(StringId32 type, CompileFunction compileFunction, float spawnOrder)
{
	ComponentTypeData componentTypeData(getDefaultAllocator());
	componentTypeData.compileFunction = compileFunction;

	ComponentTypeInfo componentTypeInfo;
	componentTypeInfo.componentType = type;
	componentTypeInfo.spawnOrder = spawnOrder;

	SortMapFn::set(componentTypeDataMap, type, componentTypeData);
	SortMapFn::sort(componentTypeDataMap);

	ArrayFn::pushBack(this->componentTypeInfoList, componentTypeInfo);
	std::sort(ArrayFn::begin(this->componentTypeInfoList), ArrayFn::end(this->componentTypeInfoList));
}

Buffer UnitCompiler::compileComponent(StringId32 type, const char* json)
{
	DATA_COMPILER_ASSERT(SortMapFn::has(componentTypeDataMap, type), this->compileOptions, "Unknown component");

	return SortMapFn::get(componentTypeDataMap, type, ComponentTypeData(getDefaultAllocator())).compileFunction(json, this->compileOptions);
}

} // namespace Rio
