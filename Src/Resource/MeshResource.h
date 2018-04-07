#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct VertexData
{
	uint32_t vertexListCount = 0;
	uint32_t stride = 0;
	char* data = nullptr;
};

struct IndexData
{
	uint32_t indexListCount = 0;
	char* data = nullptr; // size = indexListCount * sizeof(uint16_t)
};

struct MeshGeometry
{
	RioRenderer::VertexDecl vertexDecl;
	RioRenderer::VertexBufferHandle vertexBufferHandle;
	RioRenderer::IndexBufferHandle indexBufferHandle;

	Obb obb;
	VertexData vertexData;
	IndexData indexData;
};

struct MeshResource
{
	Array<StringId32> geometryNameList;
	Array<MeshGeometry*> geometryList;

	MeshResource(Allocator& a)
		: geometryNameList(a)
		, geometryList(a)
	{
	}

	const MeshGeometry* getMeshGeometry(StringId32 name) const
	{
		for (uint32_t i = 0; i < ArrayFn::getCount(geometryNameList); ++i)
		{
			if (geometryNameList[i] == name)
			{
				return geometryList[i];
			}
		}

		RIO_FATAL("Mesh name not found");
		return nullptr;
	}
};

namespace MeshResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*resourceManager*/);
	void offline(StringId64 /*id*/, ResourceManager& /*resourceManager*/);
	void unload(Allocator& a, void* resourceData);

} // namespace MeshResourceInternalFn

} // namespace Rio
