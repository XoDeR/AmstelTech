#include "Resource/MaterialResource.h"

#include "Core/Containers/Map.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/String.h"

#include "Device/Device.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#include "World/MaterialManager.h"

namespace Rio
{

namespace MaterialResourceInternalFn
{
	struct UniformTypeInfo
	{
		const char* name = nullptr;
		UniformType::Enum type = UniformType::COUNT;
		uint8_t size = 0;
	};

	static const UniformTypeInfo uniformTypeInfoMap[] =
	{
		{ "float",   UniformType::FLOAT, 4 },
		{ "vector2", UniformType::VECTOR2, 8 },
		{ "vector3", UniformType::VECTOR3, 12 },
		{ "vector4", UniformType::VECTOR4, 16 }
	};
	RIO_STATIC_ASSERT(countof(uniformTypeInfoMap) == UniformType::COUNT);

	static UniformType::Enum getUniformTypeByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(uniformTypeInfoMap); ++i)
		{
			if (strcmp(uniformTypeInfoMap[i].name, name) == 0)
			{
				return uniformTypeInfoMap[i].type;
			}
		}

		return UniformType::COUNT;
	}

	struct Data
	{
		Array<TextureData> textureList;
		Array<UniformData> uniformList;
		Array<char> dynamicBlob;

		Data()
			: textureList(getDefaultAllocator())
			, uniformList(getDefaultAllocator())
			, dynamicBlob(getDefaultAllocator())
		{
		}
	};

	// Returns offset to start of data
	template <typename T>
	static uint32_t reserveDynamicData(T data, Array<char>& dynamicBlob)
	{
		uint32_t offset = ArrayFn::getCount(dynamicBlob);
		ArrayFn::push(dynamicBlob, (char*)&data, sizeof(data));
		return offset;
	}

	static void parseTextureList(const char* json, Array<TextureData>& textureDataList, Array<char>& nameList, Array<char>& dynamicBlob, CompileOptions& compileOptions)
	{
		TempAllocator4096 tempAllocator4096;
		JsonObject textureListJsonObject(tempAllocator4096);
		RJsonFn::parse(json, textureListJsonObject);

		auto currentTextureJsonObject = JsonObjectFn::begin(textureListJsonObject);
		auto endTextureJsonObject = JsonObjectFn::end(textureListJsonObject);

		for (; currentTextureJsonObject != endTextureJsonObject; ++currentTextureJsonObject)
		{
			const FixedString key = currentTextureJsonObject->pair.first;
			const char* value = currentTextureJsonObject->pair.second;

			DynamicString texture(tempAllocator4096);
			RJsonFn::parseString(value, texture);
			DATA_COMPILER_ASSERT_RESOURCE_EXISTS("texture", texture.getCStr(), compileOptions);

			TextureHandle textureHandle;
			textureHandle.samplerHandleIndex = 0;
			textureHandle.textureHandleIndex = 0;

			const uint32_t samplerNameOffset = ArrayFn::getCount(nameList);
			ArrayFn::push(nameList, key.getCStr(), key.getLength());
			ArrayFn::pushBack(nameList, '\0');

			TextureData textureData;
			textureData.samplerNameOffset = samplerNameOffset;
			textureData.name = StringId32(key.getCStr(), key.getLength());
			textureData.id = RJsonFn::parseResourceId(value);
			textureData.dataOffset = reserveDynamicData(textureHandle, dynamicBlob);
			textureData.padding01 = 0;

			ArrayFn::pushBack(textureDataList, textureData);
		}
	}

	static void parseUniformList(const char* json, Array<UniformData>& uniformList, Array<char>& nameList, Array<char>& dynamicBlob, CompileOptions& compileOptions)
	{
		TempAllocator4096 tempAllocator4096;
		JsonObject uniformListJsonObject(tempAllocator4096);
		RJsonFn::parse(json, uniformListJsonObject);

		auto currentUniformJsonObject = JsonObjectFn::begin(uniformListJsonObject);
		auto endUniformJsonObject = JsonObjectFn::end(uniformListJsonObject);

		for (; currentUniformJsonObject != endUniformJsonObject; ++currentUniformJsonObject)
		{
			const FixedString key = currentUniformJsonObject->pair.first;
			const char* valueString = currentUniformJsonObject->pair.second;

			UniformHandle uniformHandle;

			JsonObject uniformJsonObject(tempAllocator4096);
			RJsonFn::parseObject(valueString, uniformJsonObject);

			DynamicString type(tempAllocator4096);
			RJsonFn::parseString(uniformJsonObject["type"], type);

			const UniformType::Enum uniformType = getUniformTypeByName(type.getCStr());
			DATA_COMPILER_ASSERT(uniformType != UniformType::COUNT
				, compileOptions
				, "Unknown uniform type: '%s'"
				, type.getCStr()
				);

			const uint32_t nameOffset = ArrayFn::getCount(nameList);
			ArrayFn::push(nameList, key.getCStr(), key.getLength());
			ArrayFn::pushBack(nameList, '\0');

			UniformData uniformData;
			uniformData.type = uniformType;
			uniformData.name = StringId32(key.getCStr(), key.getLength());
			uniformData.nameOffset = nameOffset;
			uniformData.dataOffset = reserveDynamicData(uniformHandle, dynamicBlob);

			switch (uniformData.type)
			{
			case UniformType::FLOAT:
				reserveDynamicData(RJsonFn::parseFloat(uniformJsonObject["value"]), dynamicBlob);
				break;

			case UniformType::VECTOR2:
				reserveDynamicData(RJsonFn::parseVector2(uniformJsonObject["value"]), dynamicBlob);
				break;

			case UniformType::VECTOR3:
				reserveDynamicData(RJsonFn::parseVector3(uniformJsonObject["value"]), dynamicBlob);
				break;

			case UniformType::VECTOR4:
				reserveDynamicData(RJsonFn::parseVector4(uniformJsonObject["value"]), dynamicBlob);
				break;

			default:
				RIO_FATAL("Unknown uniform type");
				break;
			}

			ArrayFn::pushBack(uniformList, uniformData);
		}
	}

	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();
		TempAllocator4096 tempAllocator4096;
		JsonObject materialJsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, materialJsonObject);

		Array<TextureData> textureDataList(getDefaultAllocator());
		Array<UniformData> uniformDataList(getDefaultAllocator());
		Array<char> nameList(getDefaultAllocator());
		Array<char> dynamicBlob(getDefaultAllocator());

		DynamicString shaderName(tempAllocator4096);
		RJsonFn::parseString(materialJsonObject["shader"], shaderName);

		parseTextureList(materialJsonObject["textureList"], textureDataList, nameList, dynamicBlob, compileOptions);
		parseUniformList(materialJsonObject["uniformList"], uniformDataList, nameList, dynamicBlob, compileOptions);

		MaterialResource materialResource;
		materialResource.version = RESOURCE_VERSION_MATERIAL;
		materialResource.shader = shaderName.toStringId32();
		materialResource.textureListCount = ArrayFn::getCount(textureDataList);
		materialResource.textureDataOffset = sizeof(materialResource);
		materialResource.uniformListCount = ArrayFn::getCount(uniformDataList);
		materialResource.uniformDataOffset = materialResource.textureDataOffset + sizeof(TextureData)*ArrayFn::getCount(textureDataList);
		materialResource.dynamicDataSize = ArrayFn::getCount(dynamicBlob);
		materialResource.dynamicDataOffset = materialResource.uniformDataOffset + sizeof(UniformData)*ArrayFn::getCount(uniformDataList);

		// Write
		compileOptions.write(materialResource.version);
		compileOptions.write(materialResource.shader);
		compileOptions.write(materialResource.textureListCount);
		compileOptions.write(materialResource.textureDataOffset);
		compileOptions.write(materialResource.uniformListCount);
		compileOptions.write(materialResource.uniformDataOffset);
		compileOptions.write(materialResource.dynamicDataSize);
		compileOptions.write(materialResource.dynamicDataOffset);

		for (uint32_t i = 0; i < ArrayFn::getCount(textureDataList); i++)
		{
			compileOptions.write(textureDataList[i].samplerNameOffset);
			compileOptions.write(textureDataList[i].name.id);
			compileOptions.write(textureDataList[i].id);
			compileOptions.write(textureDataList[i].dataOffset);
			compileOptions.write(textureDataList[i].padding01);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(uniformDataList); i++)
		{
			compileOptions.write(uniformDataList[i].type);
			compileOptions.write(uniformDataList[i].name);
			compileOptions.write(uniformDataList[i].nameOffset);
			compileOptions.write(uniformDataList[i].dataOffset);
		}

		compileOptions.write(dynamicBlob);
		compileOptions.write(nameList);
	}

	void* load(File& file, Allocator& a)
	{
		return getDevice()->materialManager->load(file, a);
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->materialManager->online(id, resourceManager);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->materialManager->offline(id, resourceManager);
	}

	void unload(Allocator& a, void* resourceData)
	{
		getDevice()->materialManager->unload(a, resourceData);
	}

} // namespace MaterialResourceInternalFn

namespace MaterialResourceFn
{
	UniformData* getUniformDataByIndex(const MaterialResource* materialResource, uint32_t i)
	{
		UniformData* base = (UniformData*)((char*)materialResource + materialResource->uniformDataOffset);
		return &base[i];
	}

	UniformData* getUniformDataByName(const MaterialResource* materialResource, StringId32 name)
	{
		for (uint32_t i = 0, n = materialResource->uniformListCount; i < n; ++i)
		{
			UniformData* uniformData = getUniformDataByIndex(materialResource, i);
			if (uniformData->name == name)
			{
				return uniformData;
			}
		}

		RIO_FATAL("Unknown uniform");
		return nullptr;
	}

	const char* getUniformName(const MaterialResource* materialResource, const UniformData* uniformData)
	{
		return (const char*)materialResource + materialResource->dynamicDataOffset + materialResource->dynamicDataSize + uniformData->nameOffset;
	}

	TextureData* getTextureDataByIndex(const MaterialResource* materialResource, uint32_t i)
	{
		TextureData* base = (TextureData*)((char*)materialResource + materialResource->textureDataOffset);
		return &base[i];
	}

	const char* getTextureName(const MaterialResource* materialResource, const TextureData* textureData)
	{
		return (const char*)materialResource + materialResource->dynamicDataOffset + materialResource->dynamicDataSize + textureData->samplerNameOffset;
	}

	UniformHandle* getUniformHandleByIndex(const MaterialResource* materialResource, uint32_t i, char* dynamic)
	{
		UniformData* uniformData = getUniformDataByIndex(materialResource, i);
		return (UniformHandle*) (dynamic + uniformData->dataOffset);
	}

	UniformHandle* getUniformHandleByName(const MaterialResource* materialResource, StringId32 name, char* dynamic)
	{
		UniformData* uniformData = getUniformDataByName(materialResource, name);
		return (UniformHandle*)(dynamic + uniformData->dataOffset);
	}

	TextureHandle* getTextureHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic)
	{
		TextureData* textureData = getTextureDataByIndex(materialResource, i);
		return (TextureHandle*)(dynamic + textureData->dataOffset);
	}

} // namespace MaterialResourceFn

} // namespace Rio
