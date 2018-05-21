#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "Resource/Renderer/MeshResource.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct MeshManager
{
	struct MeshData
	{
		RioRenderer::VertexBufferHandle vertexBufferHandle;
		RioRenderer::IndexBufferHandle indexBufferHandle;
	};

	struct MeshInstanceData
	{
		uint32_t size = 0;
		uint32_t capacity = 0;
		void* buffer = nullptr;

		uint32_t firstHiddenIndex = 0;

		UnitId* unitIdList = nullptr;
		const MeshResource** meshResourceList = nullptr;
		const MeshGeometry** meshGeometryList = nullptr;
		MeshData* meshDataList = nullptr;
		StringId64* materialNameList = nullptr;
		Matrix4x4* worldMatrix4x4List = nullptr;
		Obb* obbList = nullptr;
		MeshInstance* nextMeshInstanceList = nullptr;
	};

	Allocator* allocator = nullptr;
	HashMap<UnitId, uint32_t> unitIdToMeshInstanceIndexMap;
	MeshInstanceData meshInstanceData;

	MeshManager(Allocator& a)
		: allocator(&a)
		, unitIdToMeshInstanceIndexMap(a)
	{
		memset(&(this->meshInstanceData), 0, sizeof(this->meshInstanceData));
	}

	void allocate(uint32_t count);
	void grow();
	MeshInstance create(UnitId unitId, const MeshResource* meshResource, const MeshGeometry* meshGeometry, StringId64 materialName, const Matrix4x4& transformMatrix4x4);
	void destroy(MeshInstance meshInstance);
	bool has(UnitId unitId);
	MeshInstance getFirst(UnitId unitId);
	MeshInstance getNext(MeshInstance meshInstance);
	MeshInstance getPrevious(MeshInstance meshInstance);

	void addMeshNode(MeshInstance firstMeshInstance, MeshInstance meshInstance);
	void removeMeshNode(MeshInstance firstMeshInstance, MeshInstance meshInstance);
	void swapMeshNode(MeshInstance meshInstance, MeshInstance otherMeshInstance);
	void destroy();

	MeshInstance makeMeshInstance(uint32_t index)
	{
		MeshInstance meshInstance = { index };
		return meshInstance;
	}
};

} // namespace Rio