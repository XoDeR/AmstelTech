#include "Resource/PackageResource.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringId.h"

#include "Resource/CompileOptions.h"

namespace Rio
{

namespace PackageResourceInternalFn
{
	void compileResourceList(const char* type, const JsonArray& nameListJsonArray, Array<PackageResource::Resource>& output, CompileOptions& compileOptions)
	{
		const StringId64 resourceType = StringId64(type);

		for (uint32_t i = 0; i < ArrayFn::getCount(nameListJsonArray); ++i)
		{
			TempAllocator1024 tempAllocator1024;
			DynamicString name(tempAllocator1024);
			RJsonFn::parseString(nameListJsonArray[i], name);

			DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name.getCStr(), compileOptions);

			const StringId64 resourceName = RJsonFn::parseResourceId(nameListJsonArray[i]);
			ArrayFn::pushBack(output, PackageResource::Resource(resourceType, resourceName));
		}
	}

	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject packageJsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, packageJsonObject);

		JsonArray texture(tempAllocator4096);

		JsonArray shader(tempAllocator4096);
		JsonArray spriteAnimation(tempAllocator4096);
		
		JsonArray mesh(tempAllocator4096);
		JsonArray unit(tempAllocator4096);
		JsonArray sprite(tempAllocator4096);
		JsonArray material(tempAllocator4096);
		JsonArray font(tempAllocator4096);
		JsonArray level(tempAllocator4096);

#if AMSTEL_ENGINE_SCRIPT_LUA
		JsonArray script(tempAllocator4096);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SOUND
		JsonArray sound(tempAllocator4096);
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
		JsonArray physicsConfig(tempAllocator4096);
#endif // AMSTEL_ENGINE_PHYSICS
		
		if (JsonObjectFn::has(packageJsonObject, "texture"))
		{
			RJsonFn::parseArray(packageJsonObject["texture"], texture);
		}
		if (JsonObjectFn::has(packageJsonObject, "mesh"))
		{
			RJsonFn::parseArray(packageJsonObject["mesh"], mesh);
		}
		if (JsonObjectFn::has(packageJsonObject, "unit"))
		{
			RJsonFn::parseArray(packageJsonObject["unit"], unit);
		}
		if (JsonObjectFn::has(packageJsonObject, "sprite"))
		{
			RJsonFn::parseArray(packageJsonObject["sprite"], sprite);
		}
		if (JsonObjectFn::has(packageJsonObject, "material"))
		{
			RJsonFn::parseArray(packageJsonObject["material"], material);
		}
		if (JsonObjectFn::has(packageJsonObject, "font"))
		{
			RJsonFn::parseArray(packageJsonObject["font"], font);
		}
		if (JsonObjectFn::has(packageJsonObject, "level"))
		{
			RJsonFn::parseArray(packageJsonObject["level"], level);
		}
		if (JsonObjectFn::has(packageJsonObject, "shader"))
		{
			RJsonFn::parseArray(packageJsonObject["shader"], shader);
		}
		if (JsonObjectFn::has(packageJsonObject, "spriteAnimation"))
		{
			RJsonFn::parseArray(packageJsonObject["spriteAnimation"], spriteAnimation);
		}

#if AMSTEL_ENGINE_SCRIPT_LUA
		if (JsonObjectFn::has(packageJsonObject, "lua"))
		{
			RJsonFn::parseArray(packageJsonObject["lua"], script);
		}
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SOUND
		if (JsonObjectFn::has(packageJsonObject, "sound"))
		{
			RJsonFn::parseArray(packageJsonObject["sound"], sound);
		}
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
		if (JsonObjectFn::has(packageJsonObject, "physicsConfig"))
		{
			RJsonFn::parseArray(packageJsonObject["physicsConfig"], physicsConfig);
		}
#endif // AMSTEL_ENGINE_PHYSICS

		Array<PackageResource::Resource> packageResourceList(getDefaultAllocator());

		compileResourceList("texture", texture, packageResourceList, compileOptions);
		compileResourceList("mesh", mesh, packageResourceList, compileOptions);
		compileResourceList("unit", unit, packageResourceList, compileOptions);
		compileResourceList("sprite", sprite, packageResourceList, compileOptions);
		compileResourceList("material", material, packageResourceList, compileOptions);
		compileResourceList("font", font, packageResourceList, compileOptions);
		compileResourceList("level", level, packageResourceList, compileOptions);
		compileResourceList("shader", shader, packageResourceList, compileOptions);
		compileResourceList("spriteAnimation", spriteAnimation, packageResourceList, compileOptions);

#if AMSTEL_ENGINE_SCRIPT_LUA
		compileResourceList("lua", script, packageResourceList, compileOptions);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SOUND
		compileResourceList("sound", sound, packageResourceList, compileOptions);
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
		compileResourceList("physicsConfig", physicsConfig, packageResourceList, compileOptions);
#endif // AMSTEL_ENGINE_PHYSICS

		// Write
		compileOptions.write(RESOURCE_VERSION_PACKAGE);
		compileOptions.write(ArrayFn::getCount(packageResourceList));

		for (uint32_t i = 0; i < ArrayFn::getCount(packageResourceList); ++i)
		{
			compileOptions.write(packageResourceList[i].type);
			compileOptions.write(packageResourceList[i].name);
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version = UINT32_MAX;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_PACKAGE, "Wrong version");

		uint32_t resourceListCount = 0;
		binaryReader.read(resourceListCount);

		PackageResource* packageResource = RIO_NEW(a, PackageResource)(a);
		ArrayFn::resize(packageResource->resourceList, resourceListCount);
		binaryReader.read(ArrayFn::begin(packageResource->resourceList), sizeof(PackageResource::Resource) * resourceListCount);

		return packageResource;
	}

	void unload(Allocator& a, void* resourceData)
	{
		RIO_DELETE(a, (PackageResource*)resourceData);
	}

} // namespace PackageResourceInternalFn

} // namespace Rio
