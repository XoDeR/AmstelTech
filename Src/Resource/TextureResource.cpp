#include "Resource/TextureResource.h"

#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#if RIO_DEVELOPMENT
	#define TEXTURE_COMPILER_NAME "TextureCompilerDevelopment"
#elif RIO_DEBUG
	#define TEXTURE_COMPILER_NAME "TextureCompilerDebug"
#else
	#define TEXTURE_COMPILER_NAME "TextureCompilerRelease"
#endif  // RIO_DEBUG

#if RIO_PLATFORM_LINUX
	#define TEXTURE_COMPILER_PATH "./" TEXTURE_COMPILER_NAME ""
#elif RIO_PLATFORM_WINDOWS
	#define TEXTURE_COMPILER_PATH TEXTURE_COMPILER_NAME ".exe"
#else
	#define TEXTURE_COMPILER_PATH ""
#endif // RIO_PLATFORM_LINUX

namespace Rio
{

namespace TextureResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject jsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, jsonObject);

		DynamicString name(tempAllocator4096);
		RJsonFn::parseString(jsonObject["source"], name);
		DATA_COMPILER_ASSERT_FILE_EXISTS(name.getCStr(), compileOptions);

		const bool generateMips = RJsonFn::parseBool(jsonObject["generateMips"]);
		const bool normalMap = RJsonFn::parseBool(jsonObject["normalMap"]);

		DynamicString textureSource(tempAllocator4096);
		DynamicString textureOutput(tempAllocator4096);
		compileOptions.getAbsolutePath(name.getCStr(), textureSource);
		compileOptions.getTemporaryPath("ktx", textureOutput); // ktx -- Khronos Texture

		const char* argumentList[] =
		{
			TEXTURE_COMPILER_PATH,
			"-f",
			textureSource.getCStr(),
			"-o",
			textureOutput.getCStr(),
			(generateMips ? "-m" : ""),
			(normalMap ? "-n" : ""),
			nullptr
		};

		StringStream output(tempAllocator4096);
		int externalCompilerResult = compileOptions.runExternalCompiler(argumentList, output);
		DATA_COMPILER_ASSERT(externalCompilerResult == 0
			, compileOptions
			, "Failed to compile texture:\n%s"
			, StringStreamFn::getCStr(output)
			);

		Buffer blob = compileOptions.readTemporary(textureOutput.getCStr());
		compileOptions.deleteFile(textureOutput.getCStr());

		// Write DDS
		compileOptions.write(RESOURCE_VERSION_TEXTURE);
		compileOptions.write(ArrayFn::getCount(blob));
		compileOptions.write(blob);
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_TEXTURE, "Wrong version");

		uint32_t size;
		binaryReader.read(size);

		TextureResource* textureResource = (TextureResource*)a.allocate(sizeof(TextureResource) + size);

		void* data = &textureResource[1];
		binaryReader.read(data, size);

		textureResource->memory = RioRenderer::makeRef(data, size);
		textureResource->handle.idx = RIO_RENDERER_INVALID_HANDLE_UINT16_T;

		return textureResource;
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		TextureResource* textureResource = (TextureResource*)resourceManager.get(RESOURCE_TYPE_TEXTURE, id);
		textureResource->handle = RioRenderer::createTexture(textureResource->memory);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		TextureResource* textureResource = (TextureResource*)resourceManager.get(RESOURCE_TYPE_TEXTURE, id);
		RioRenderer::destroy(textureResource->handle);
	}

	void unload(Allocator& a, void* resourceData)
	{
		a.deallocate(resourceData);
	}

} // namespace TextureResourceInternalFn

} // namespace Rio
