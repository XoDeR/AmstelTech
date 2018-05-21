#include "World/Renderer/MeshManager.h"

#include "Core/Containers/HashMap.h"

namespace Rio
{

void MeshManager::allocate(uint32_t count)
{
	RIO_ENSURE(count > this->meshInstanceData.size);

	const uint32_t bytes = 0
		+ count * sizeof(UnitId) + alignof(UnitId)
		+ count * sizeof(MeshResource*) + alignof(MeshResource*)
		+ count * sizeof(MeshGeometry*) + alignof(MeshGeometry*)
		+ count * sizeof(MeshData) + alignof(MeshData)
		+ count * sizeof(StringId64) + alignof(StringId64)
		+ count * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ count * sizeof(Obb) + alignof(Obb)
		+ count * sizeof(MeshInstance) + alignof(MeshInstance)
		;

	MeshInstanceData newMeshInstanceData;
	newMeshInstanceData.size = this->meshInstanceData.size;
	newMeshInstanceData.capacity = count;
	newMeshInstanceData.buffer = allocator->allocate(bytes);
	newMeshInstanceData.firstHiddenIndex = this->meshInstanceData.firstHiddenIndex;

	newMeshInstanceData.unitIdList = (UnitId*)newMeshInstanceData.buffer;
	newMeshInstanceData.meshResourceList = (const MeshResource**)Memory::getAlignedToTop(newMeshInstanceData.unitIdList + count, alignof(const MeshResource*));
	newMeshInstanceData.meshGeometryList = (const MeshGeometry**)Memory::getAlignedToTop(newMeshInstanceData.meshResourceList + count, alignof(const MeshGeometry*));
	newMeshInstanceData.meshDataList = (MeshData*)Memory::getAlignedToTop(newMeshInstanceData.meshGeometryList + count, alignof(MeshData));
	newMeshInstanceData.materialNameList = (StringId64*)Memory::getAlignedToTop(newMeshInstanceData.meshDataList + count, alignof(StringId64));
	newMeshInstanceData.worldMatrix4x4List = (Matrix4x4*)Memory::getAlignedToTop(newMeshInstanceData.materialNameList + count, alignof(Matrix4x4));
	newMeshInstanceData.obbList = (Obb*)Memory::getAlignedToTop(newMeshInstanceData.worldMatrix4x4List + count, alignof(Obb));
	newMeshInstanceData.nextMeshInstanceList = (MeshInstance*)Memory::getAlignedToTop(newMeshInstanceData.obbList + count, alignof(MeshInstance));

	memcpy(newMeshInstanceData.unitIdList, this->meshInstanceData.unitIdList, this->meshInstanceData.size * sizeof(UnitId));
	memcpy(newMeshInstanceData.meshResourceList, this->meshInstanceData.meshResourceList, this->meshInstanceData.size * sizeof(MeshResource*));
	memcpy(newMeshInstanceData.meshGeometryList, this->meshInstanceData.meshGeometryList, this->meshInstanceData.size * sizeof(MeshGeometry*));
	memcpy(newMeshInstanceData.meshDataList, this->meshInstanceData.meshDataList, this->meshInstanceData.size * sizeof(MeshData));
	memcpy(newMeshInstanceData.materialNameList, this->meshInstanceData.materialNameList, this->meshInstanceData.size * sizeof(StringId64));
	memcpy(newMeshInstanceData.worldMatrix4x4List, this->meshInstanceData.worldMatrix4x4List, this->meshInstanceData.size * sizeof(Matrix4x4));
	memcpy(newMeshInstanceData.obbList, this->meshInstanceData.obbList, this->meshInstanceData.size * sizeof(Obb));
	memcpy(newMeshInstanceData.nextMeshInstanceList, this->meshInstanceData.nextMeshInstanceList, this->meshInstanceData.size * sizeof(MeshInstance));

	allocator->deallocate(this->meshInstanceData.buffer);
	this->meshInstanceData = newMeshInstanceData;
}

void MeshManager::grow()
{
	allocate(this->meshInstanceData.capacity * 2 + 1);
}

MeshInstance MeshManager::create(UnitId unitId, const MeshResource* meshResource, const MeshGeometry* meshGeometry, StringId64 materialName, const Matrix4x4& transformMatrix4x4)
{
	if (this->meshInstanceData.size == this->meshInstanceData.capacity)
	{
		grow();
	}

	const uint32_t lastIndex = this->meshInstanceData.size;

	this->meshInstanceData.unitIdList[lastIndex] = unitId;
	this->meshInstanceData.meshResourceList[lastIndex] = meshResource;
	this->meshInstanceData.meshGeometryList[lastIndex] = meshGeometry;
	this->meshInstanceData.meshDataList[lastIndex].vertexBufferHandle = meshGeometry->vertexBufferHandle;
	this->meshInstanceData.meshDataList[lastIndex].indexBufferHandle = meshGeometry->indexBufferHandle;
	this->meshInstanceData.materialNameList[lastIndex] = materialName;
	this->meshInstanceData.worldMatrix4x4List[lastIndex] = transformMatrix4x4;
	this->meshInstanceData.obbList[lastIndex] = meshGeometry->obb;
	this->meshInstanceData.nextMeshInstanceList[lastIndex] = makeMeshInstance(UINT32_MAX);

	++this->meshInstanceData.size;
	++this->meshInstanceData.firstHiddenIndex;

	MeshInstance currentMeshInstance = getFirst(unitId);
	if (!getIsValid(currentMeshInstance))
	{
		HashMapFn::set(this->unitIdToMeshInstanceIndexMap, unitId, lastIndex);
	}
	else
	{
		addMeshNode(currentMeshInstance, makeMeshInstance(lastIndex));
	}

	return makeMeshInstance(lastIndex);
}

void MeshManager::destroy(MeshInstance meshInstance)
{
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");

	const uint32_t lastIndex = this->meshInstanceData.size - 1;
	const UnitId unitId = this->meshInstanceData.unitIdList[meshInstance.index];
	const MeshInstance firstMeshInstance = getFirst(unitId);
	const MeshInstance lastMeshInstance = makeMeshInstance(lastIndex);

	swapMeshNode(lastMeshInstance, meshInstance);
	removeMeshNode(firstMeshInstance, meshInstance);

	this->meshInstanceData.unitIdList[meshInstance.index] = this->meshInstanceData.unitIdList[lastIndex];
	this->meshInstanceData.meshResourceList[meshInstance.index] = this->meshInstanceData.meshResourceList[lastIndex];
	this->meshInstanceData.meshGeometryList[meshInstance.index] = this->meshInstanceData.meshGeometryList[lastIndex];
	this->meshInstanceData.meshDataList[meshInstance.index].vertexBufferHandle = this->meshInstanceData.meshDataList[lastIndex].vertexBufferHandle;
	this->meshInstanceData.meshDataList[meshInstance.index].indexBufferHandle = this->meshInstanceData.meshDataList[lastIndex].indexBufferHandle;
	this->meshInstanceData.materialNameList[meshInstance.index] = this->meshInstanceData.materialNameList[lastIndex];
	this->meshInstanceData.worldMatrix4x4List[meshInstance.index] = this->meshInstanceData.worldMatrix4x4List[lastIndex];
	this->meshInstanceData.obbList[meshInstance.index] = this->meshInstanceData.obbList[lastIndex];
	this->meshInstanceData.nextMeshInstanceList[meshInstance.index] = this->meshInstanceData.nextMeshInstanceList[lastIndex];

	--this->meshInstanceData.size;
	--this->meshInstanceData.firstHiddenIndex;
}

bool MeshManager::has(UnitId unitId)
{
	return getIsValid(getFirst(unitId));
}

MeshInstance MeshManager::getFirst(UnitId unitId)
{
	return makeMeshInstance(HashMapFn::get(this->unitIdToMeshInstanceIndexMap, unitId, UINT32_MAX));
}

MeshInstance MeshManager::getNext(MeshInstance meshInstance)
{
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");
	return this->meshInstanceData.nextMeshInstanceList[meshInstance.index];
}

MeshInstance MeshManager::getPrevious(MeshInstance meshInstance)
{
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");

	const UnitId unitId = this->meshInstanceData.unitIdList[meshInstance.index];

	MeshInstance currentMeshInstance = getFirst(unitId);
	MeshInstance previousMeshInstance = { UINT32_MAX };

	while (currentMeshInstance.index != meshInstance.index)
	{
		previousMeshInstance = currentMeshInstance;
		currentMeshInstance = getNext(currentMeshInstance);
	}

	return previousMeshInstance;
}

void MeshManager::addMeshNode(MeshInstance firstMeshInstance, MeshInstance meshInstance)
{
	RIO_ASSERT(firstMeshInstance.index < this->meshInstanceData.size, "Index out of bounds");
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");

	MeshInstance currentMeshInstance = firstMeshInstance;
	while (getIsValid(getNext(currentMeshInstance)))
	{
		currentMeshInstance = getNext(currentMeshInstance);
	}

	this->meshInstanceData.nextMeshInstanceList[currentMeshInstance.index] = meshInstance;
}

void MeshManager::removeMeshNode(MeshInstance firstMeshInstance, MeshInstance meshInstance)
{
	RIO_ASSERT(firstMeshInstance.index < this->meshInstanceData.size, "Index out of bounds");
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");

	const UnitId unitId = this->meshInstanceData.unitIdList[firstMeshInstance.index];

	if (meshInstance.index == firstMeshInstance.index)
	{
		if (!getIsValid(getNext(meshInstance)))
		{
			HashMapFn::remove(this->unitIdToMeshInstanceIndexMap, unitId);
		}
		else
		{
			HashMapFn::set(this->unitIdToMeshInstanceIndexMap, unitId, getNext(meshInstance).index);
		}
	}
	else
	{
		MeshInstance previousMeshInstance = getPrevious(meshInstance);
		this->meshInstanceData.nextMeshInstanceList[previousMeshInstance.index] = getNext(meshInstance);
	}
}

void MeshManager::swapMeshNode(MeshInstance meshInstance, MeshInstance otherMeshInstance)
{
	RIO_ASSERT(meshInstance.index < this->meshInstanceData.size, "Index out of bounds");
	RIO_ASSERT(otherMeshInstance.index < this->meshInstanceData.size, "Index out of bounds");

	const UnitId unitId = this->meshInstanceData.unitIdList[meshInstance.index];
	const MeshInstance firstMeshInstance = getFirst(unitId);

	if (meshInstance.index == firstMeshInstance.index)
	{
		HashMapFn::set(this->unitIdToMeshInstanceIndexMap, unitId, otherMeshInstance.index);
	}
	else
	{
		const MeshInstance previousMeshInstance = getPrevious(meshInstance);
		RIO_ENSURE(previousMeshInstance.index != meshInstance.index);
		this->meshInstanceData.nextMeshInstanceList[previousMeshInstance.index] = otherMeshInstance;
	}
}

void MeshManager::destroy()
{
	allocator->deallocate(this->meshInstanceData.buffer);
}

} // namespace Rio