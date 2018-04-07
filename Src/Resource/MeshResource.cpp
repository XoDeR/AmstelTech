#include "Resource/MeshResource.h"

#include "Core/Containers/Map.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Device/DeviceLog.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"
#include "Resource/MeshCompiler.h"

namespace Rio
{

namespace MeshResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject jsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, jsonObject);

		JsonObject geometryList(tempAllocator4096);
		RJsonFn::parse(jsonObject["geometryList"], geometryList);
		JsonObject nodeList(tempAllocator4096);
		RJsonFn::parse(jsonObject["nodeList"], nodeList);

		compileOptions.write(RESOURCE_VERSION_MESH);
		compileOptions.write(JsonObjectFn::getSize(geometryList));

		MeshCompiler meshCompiler(compileOptions);

		auto currentGeometryJson = JsonObjectFn::begin(geometryList);
		auto endGeometryJson = JsonObjectFn::end(geometryList);
		for (; currentGeometryJson != endGeometryJson; ++currentGeometryJson)
		{
			const FixedString key = currentGeometryJson->pair.first;
			const char* geometry = currentGeometryJson->pair.second;
			const char* node = nodeList[key];

			const StringId32 name(key.getCStr(), key.getLength());
			compileOptions.write(name.id);

			meshCompiler.reset();
			meshCompiler.parse(geometry, node);
			meshCompiler.compile();
			meshCompiler.write();
		}
	}

	void* load(File& file, Allocator& a)
	{
		BinaryReader binaryReader(file);

		uint32_t version;
		binaryReader.read(version);
		RIO_ASSERT(version == RESOURCE_VERSION_MESH, "Wrong mesh resource version");

		uint32_t geometryCount = 0;
		binaryReader.read(geometryCount);

		MeshResource* meshResource = RIO_NEW(a, MeshResource)(a);
		ArrayFn::resize(meshResource->geometryNameList, geometryCount);
		ArrayFn::resize(meshResource->geometryList, geometryCount);

		for (uint32_t i = 0; i < geometryCount; ++i)
		{
			StringId32 name;
			binaryReader.read(name);

			RioRenderer::VertexDecl vertexDecl;
			binaryReader.read(vertexDecl);

			Obb obb;
			binaryReader.read(obb);

			uint32_t vertexListCount = 0;
			binaryReader.read(vertexListCount);

			uint32_t stride = 0;
			binaryReader.read(stride);

			uint32_t indexListCount = 0;
			binaryReader.read(indexListCount);

			const uint32_t vertexListSize = vertexListCount * stride;
			const uint32_t indexListSize = indexListCount * sizeof(uint16_t);

			const uint32_t size = sizeof(MeshGeometry) + vertexListSize + indexListSize;

			MeshGeometry* meshGeometry = (MeshGeometry*)a.allocate(size);
			meshGeometry->obb = obb;
			meshGeometry->vertexDecl = vertexDecl;
			meshGeometry->vertexBufferHandle;
			meshGeometry->indexBufferHandle;
			meshGeometry->vertexData.vertexListCount = vertexListCount;
			meshGeometry->vertexData.stride = stride;
			meshGeometry->vertexData.data = (char*)&meshGeometry[1];
			meshGeometry->indexData.indexListCount = indexListCount;
			meshGeometry->indexData.data = meshGeometry->vertexData.data + vertexListSize;

			binaryReader.read(meshGeometry->vertexData.data, vertexListSize);
			binaryReader.read(meshGeometry->indexData.data, indexListSize);

			meshResource->geometryNameList[i] = name;
			meshResource->geometryList[i] = meshGeometry;
		}

		return meshResource;
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		MeshResource* meshResource = (MeshResource*)resourceManager.getResourceData(RESOURCE_TYPE_MESH, id);

		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->geometryList); ++i)
		{
			MeshGeometry& meshGeometry = *meshResource->geometryList[i];

			const uint32_t vertexListSize = meshGeometry.vertexData.vertexListCount * meshGeometry.vertexData.stride;
			const uint32_t indexListSize = meshGeometry.indexData.indexListCount * sizeof(uint16_t);

			const RioRenderer::Memory* vertexListMemory = RioRenderer::makeRef(meshGeometry.vertexData.data, vertexListSize);
			const RioRenderer::Memory* indexListMemory = RioRenderer::makeRef(meshGeometry.indexData.data, indexListSize);

			RioRenderer::VertexBufferHandle vertexBufferHandle = RioRenderer::createVertexBuffer(vertexListMemory, meshGeometry.vertexDecl);
			RioRenderer::IndexBufferHandle indexBufferHandle = RioRenderer::createIndexBuffer(indexListMemory);
			RIO_ASSERT(RioRenderer::isValid(vertexBufferHandle), "Invalid vertex buffer");
			RIO_ASSERT(RioRenderer::isValid(indexBufferHandle), "Invalid index buffer");

			meshGeometry.vertexBufferHandle = vertexBufferHandle;
			meshGeometry.indexBufferHandle = indexBufferHandle;
		}
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		MeshResource* meshResource = (MeshResource*)resourceManager.getResourceData(RESOURCE_TYPE_MESH, id);

		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->geometryList); ++i)
		{
			MeshGeometry& meshGeometry = *meshResource->geometryList[i];
			RioRenderer::destroy(meshGeometry.vertexBufferHandle);
			RioRenderer::destroy(meshGeometry.indexBufferHandle);
		}
	}

	void unload(Allocator& a, void* resourceData)
	{
		MeshResource* meshResource = (MeshResource*)resourceData;

		for (uint32_t i = 0; i < ArrayFn::getCount(meshResource->geometryList); ++i)
		{
			a.deallocate(meshResource->geometryList[i]);
		}
		RIO_DELETE(a, (MeshResource*)resourceData);
	}

} // namespace MeshResourceInternalFn

} // namespace Rio
