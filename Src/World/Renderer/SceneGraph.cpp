#include "World/Renderer/SceneGraph.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"
#include "Core/Memory/Allocator.h"

#include "World/UnitManager.h"

#include <cstdint> // UINT_MAX
#include <cstring> // memcpy

namespace Rio
{

static void unitDestroyedCallbackBridge(UnitId unitId, void* userPtr)
{
	((SceneGraph*)userPtr)->unitDestroyedCallback(unitId);
}

SceneGraph::Pose& SceneGraph::Pose::operator=(const Matrix4x4& matrix4x4)
{
	Matrix3x3 rotationMatrix3x3 = getRotationMatrix3x3Truncated(matrix4x4);
	getNormalized(rotationMatrix3x3.x);
	getNormalized(rotationMatrix3x3.y);
	getNormalized(rotationMatrix3x3.z);

	position = getTranslation(matrix4x4);
	rotation = rotationMatrix3x3;
	scale = Rio::getScale(matrix4x4);
	return *this;
}

SceneGraph::SceneGraph(Allocator& a, UnitManager& unitManager)
	: allocator(&a)
	, unitManager(&unitManager)
	, unitIdToTransformInstanceMap(a)
{
	unitManager.registerDestroyFunction(unitDestroyedCallbackBridge, this);
}

SceneGraph::~SceneGraph()
{
	unitManager->unregisterDestroyFunction(this);

	allocator->deallocate(this->sceneGraphInstanceData.buffer);

	marker = 0;
}

TransformInstance SceneGraph::makeTransformInstance(uint32_t index)
{
	TransformInstance transformInstance = { index };
	return transformInstance;
}

void SceneGraph::allocate(uint32_t instanceDataSize)
{
	RIO_ASSERT(instanceDataSize > this->sceneGraphInstanceData.size, "instanceDataSize > this->sceneGraphInstanceData.size");

	const uint32_t bytes = 0
		+ instanceDataSize * sizeof(UnitId) + alignof(UnitId)
		+ instanceDataSize * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ instanceDataSize * sizeof(Pose) + alignof(Pose)
		+ instanceDataSize * sizeof(TransformInstance) * 4 + alignof(TransformInstance)
		+ instanceDataSize * sizeof(bool) + alignof(bool)
		;

	SceneGraphInstanceData newSceneGraphInstanceData;
	newSceneGraphInstanceData.size = this->sceneGraphInstanceData.size;
	newSceneGraphInstanceData.capacity = instanceDataSize;
	newSceneGraphInstanceData.buffer = allocator->allocate(bytes);

	newSceneGraphInstanceData.unitIdList = (UnitId*)newSceneGraphInstanceData.buffer;
	newSceneGraphInstanceData.worldMatrix4x4List = (Matrix4x4*)Memory::getAlignedToTop(newSceneGraphInstanceData.unitIdList + instanceDataSize, alignof(Matrix4x4));
	newSceneGraphInstanceData.localPoseList = (Pose*)Memory::getAlignedToTop(newSceneGraphInstanceData.worldMatrix4x4List + instanceDataSize, alignof(Pose));
	newSceneGraphInstanceData.parentTransformInstanceList = (TransformInstance*)Memory::getAlignedToTop(newSceneGraphInstanceData.localPoseList + instanceDataSize, alignof(TransformInstance));
	newSceneGraphInstanceData.firstChildTransformInstanceList = (TransformInstance*)Memory::getAlignedToTop(newSceneGraphInstanceData.parentTransformInstanceList + instanceDataSize, alignof(TransformInstance));
	newSceneGraphInstanceData.nextSiblingTransformInstanceList = (TransformInstance*)Memory::getAlignedToTop(newSceneGraphInstanceData.firstChildTransformInstanceList + instanceDataSize, alignof(TransformInstance));
	newSceneGraphInstanceData.previousSiblingTransformInstanceList = (TransformInstance*)Memory::getAlignedToTop(newSceneGraphInstanceData.nextSiblingTransformInstanceList + instanceDataSize, alignof(TransformInstance));
	newSceneGraphInstanceData.hasChangedList = (bool*)Memory::getAlignedToTop(newSceneGraphInstanceData.previousSiblingTransformInstanceList + instanceDataSize, alignof(bool));

	memcpy(newSceneGraphInstanceData.unitIdList, this->sceneGraphInstanceData.unitIdList, this->sceneGraphInstanceData.size * sizeof(UnitId));
	memcpy(newSceneGraphInstanceData.worldMatrix4x4List, this->sceneGraphInstanceData.worldMatrix4x4List, this->sceneGraphInstanceData.size * sizeof(Matrix4x4));
	memcpy(newSceneGraphInstanceData.localPoseList, this->sceneGraphInstanceData.localPoseList, this->sceneGraphInstanceData.size * sizeof(Pose));
	memcpy(newSceneGraphInstanceData.parentTransformInstanceList, this->sceneGraphInstanceData.parentTransformInstanceList, this->sceneGraphInstanceData.size * sizeof(TransformInstance));
	memcpy(newSceneGraphInstanceData.firstChildTransformInstanceList, this->sceneGraphInstanceData.firstChildTransformInstanceList, this->sceneGraphInstanceData.size * sizeof(TransformInstance));
	memcpy(newSceneGraphInstanceData.nextSiblingTransformInstanceList, this->sceneGraphInstanceData.nextSiblingTransformInstanceList, this->sceneGraphInstanceData.size * sizeof(TransformInstance));
	memcpy(newSceneGraphInstanceData.previousSiblingTransformInstanceList, this->sceneGraphInstanceData.previousSiblingTransformInstanceList, this->sceneGraphInstanceData.size * sizeof(TransformInstance));
	memcpy(newSceneGraphInstanceData.hasChangedList, this->sceneGraphInstanceData.hasChangedList, this->sceneGraphInstanceData.size * sizeof(bool));

	allocator->deallocate(this->sceneGraphInstanceData.buffer);
	this->sceneGraphInstanceData = newSceneGraphInstanceData;
}

void SceneGraph::unitDestroyedCallback(UnitId unitId)
{
	if (has(unitId))
	{
		destroy(unitId, makeTransformInstance(UINT32_MAX));
	}
}

TransformInstance SceneGraph::create(UnitId unitId, const Vector3& pos, const Quaternion& rotation, const Vector3& scale)
{
	Matrix4x4 pose;
	Rio::setToIdentity(pose);
	Rio::setTranslation(pose, pos);
	Rio::setRotation(pose, rotation);
	Rio::setScale(pose, scale);

	return create(unitId, pose);
}

TransformInstance SceneGraph::create(UnitId unitId, const Matrix4x4& pose)
{
	RIO_ASSERT(!HashMapFn::has(unitIdToTransformInstanceMap, unitId), "Unit already has transform");

	if (this->sceneGraphInstanceData.capacity == this->sceneGraphInstanceData.size)
	{
		grow();
	}

	const uint32_t lastIndex = this->sceneGraphInstanceData.size;

	this->sceneGraphInstanceData.unitIdList[lastIndex] = unitId;
	this->sceneGraphInstanceData.worldMatrix4x4List[lastIndex] = pose;
	this->sceneGraphInstanceData.localPoseList[lastIndex] = pose;
	this->sceneGraphInstanceData.parentTransformInstanceList[lastIndex].index = UINT32_MAX;
	this->sceneGraphInstanceData.firstChildTransformInstanceList[lastIndex].index = UINT32_MAX;
	this->sceneGraphInstanceData.nextSiblingTransformInstanceList[lastIndex].index = UINT32_MAX;
	this->sceneGraphInstanceData.previousSiblingTransformInstanceList[lastIndex].index = UINT32_MAX;
	this->sceneGraphInstanceData.hasChangedList[lastIndex] = false;

	++this->sceneGraphInstanceData.size;

	HashMapFn::set(unitIdToTransformInstanceMap, unitId, lastIndex);

	return makeTransformInstance(lastIndex);
}

void SceneGraph::destroy(UnitId unitId, TransformInstance /*unitTransformInstance*/)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");

	const uint32_t lastIndex = this->sceneGraphInstanceData.size - 1;
	const UnitId unitIdLocal = this->sceneGraphInstanceData.unitIdList[transformInstance.index];
	const UnitId lastUnitId = this->sceneGraphInstanceData.unitIdList[lastIndex];

	this->sceneGraphInstanceData.unitIdList[transformInstance.index] = this->sceneGraphInstanceData.unitIdList[lastIndex];
	this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index] = this->sceneGraphInstanceData.worldMatrix4x4List[lastIndex];
	this->sceneGraphInstanceData.localPoseList[transformInstance.index] = this->sceneGraphInstanceData.localPoseList[lastIndex];
	this->sceneGraphInstanceData.parentTransformInstanceList[transformInstance.index] = this->sceneGraphInstanceData.parentTransformInstanceList[lastIndex];
	this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstance.index]  = this->sceneGraphInstanceData.firstChildTransformInstanceList[lastIndex];
	this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstance.index] = this->sceneGraphInstanceData.nextSiblingTransformInstanceList[lastIndex];
	this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstance.index] = this->sceneGraphInstanceData.previousSiblingTransformInstanceList[lastIndex];
	this->sceneGraphInstanceData.hasChangedList[transformInstance.index] = this->sceneGraphInstanceData.hasChangedList[lastIndex];

	HashMapFn::set(unitIdToTransformInstanceMap, lastUnitId, transformInstance.index);
	HashMapFn::remove(unitIdToTransformInstanceMap, unitIdLocal);

	--this->sceneGraphInstanceData.size;
}

TransformInstance SceneGraph::getTransformInstanceByUnitId(UnitId unitId)
{
	return makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
}

bool SceneGraph::has(UnitId unitId)
{
	return HashMapFn::has(unitIdToTransformInstanceMap, unitId);
}

void SceneGraph::setLocalPosition(UnitId unitId, const Vector3& position)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	this->sceneGraphInstanceData.localPoseList[transformInstance.index].position = position;
	setLocal(transformInstance);
}

void SceneGraph::setLocalRotation(UnitId unitId, const Quaternion& rotation)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	this->sceneGraphInstanceData.localPoseList[transformInstance.index].rotation = createMatrix3x3(rotation);
	setLocal(transformInstance);
}

void SceneGraph::setLocalScale(UnitId unitId, const Vector3& scale)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	this->sceneGraphInstanceData.localPoseList[transformInstance.index].scale = scale;
	setLocal(transformInstance);
}

void SceneGraph::setLocalPose(UnitId unitId, const Matrix4x4& pose)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	this->sceneGraphInstanceData.localPoseList[transformInstance.index] = pose;
	setLocal(transformInstance);
}

Vector3 SceneGraph::getLocalPosition(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	return this->sceneGraphInstanceData.localPoseList[transformInstance.index].position;
}

Quaternion SceneGraph::getLocalRotation(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	return createQuaternion(this->sceneGraphInstanceData.localPoseList[transformInstance.index].rotation);
}

Vector3 SceneGraph::getLocalScale(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	return this->sceneGraphInstanceData.localPoseList[transformInstance.index].scale;
}

Matrix4x4 SceneGraph::getLocalPose(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	Matrix4x4 transformMatrix4x4 = createMatrix4x4(createQuaternion(this->sceneGraphInstanceData.localPoseList[transformInstance.index].rotation), this->sceneGraphInstanceData.localPoseList[transformInstance.index].position);
	setScale(transformMatrix4x4, this->sceneGraphInstanceData.localPoseList[transformInstance.index].scale);
	return transformMatrix4x4;
}

Vector3 SceneGraph::getWorldPosition(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.i < this->sceneGraphInstanceData.size, "Index out of bounds");
	return getTranslation(this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index]);
}

Quaternion SceneGraph::getWorldRotation(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.i < this->sceneGraphInstanceData.size, "Index out of bounds");
	return getRotationQuaternion(this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index]);
}

Matrix4x4 SceneGraph::getWorldPose(UnitId unitId)
{
	TransformInstance transformInstance = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstance.i < this->sceneGraphInstanceData.size, "Index out of bounds");
	return this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index];
}

void SceneGraph::setWorldPose(TransformInstance transformInstance, const Matrix4x4& pose)
{
	RIO_ASSERT(transformInstance.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index] = pose;
	this->sceneGraphInstanceData.hasChangedList[transformInstance.index] = true;
}

uint32_t SceneGraph::getNodeListCount() const
{
	return this->sceneGraphInstanceData.size;
}

void SceneGraph::link(UnitId childUnitId, UnitId parentUnitId)
{
	TransformInstance transformInstanceChild = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, childUnitId, UINT32_MAX));
	TransformInstance transformInstanceParent = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, parentUnitId, UINT32_MAX));

	RIO_ASSERT(transformInstanceChild.index < this->sceneGraphInstanceData.size, "Index out of bounds");
	RIO_ASSERT(transformInstanceParent.index < this->sceneGraphInstanceData.size, "Index out of bounds");

	unlink(childUnitId);

	if (!getIsValid(this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstanceParent.index]))
	{
		this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstanceParent.index] = transformInstanceChild;
		this->sceneGraphInstanceData.parentTransformInstanceList[transformInstanceChild.index] = transformInstanceParent;
	}
	else
	{
		// iterate children tree
		TransformInstance previousTransformInstance = { UINT32_MAX };
		TransformInstance currentTransformInstanceChild = this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstanceParent.index];
		while (getIsValid(currentTransformInstanceChild))
		{
			previousTransformInstance = currentTransformInstanceChild;
			currentTransformInstanceChild = this->sceneGraphInstanceData.nextSiblingTransformInstanceList[currentTransformInstanceChild.index];
		}

		this->sceneGraphInstanceData.nextSiblingTransformInstanceList[previousTransformInstance.index] = transformInstanceChild;

		this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstanceChild.index].index = UINT32_MAX;
		this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index].index = UINT32_MAX;
		this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstanceChild.index] = previousTransformInstance;
	}

	Matrix4x4 parentTransformMatrix4x4 = this->sceneGraphInstanceData.worldMatrix4x4List[transformInstanceParent.index];
	Matrix4x4 childTransformMatrix4x4 = this->sceneGraphInstanceData.worldMatrix4x4List[transformInstanceChild.index];
	const Vector3 childScale = Rio::getScale(childTransformMatrix4x4);

	Vector3 parentXAxis = getXAxis(parentTransformMatrix4x4);
	Vector3 parentYAxis = getYAxis(parentTransformMatrix4x4);
	Vector3 parentZAxis = getZAxis(parentTransformMatrix4x4);
	Vector3 childXAxis = getXAxis(childTransformMatrix4x4);
	Vector3 childYAxis = getYAxis(childTransformMatrix4x4);
	Vector3 childZAxis = getZAxis(childTransformMatrix4x4);

	setXAxis(parentTransformMatrix4x4, getNormalized(parentXAxis));
	setYAxis(parentTransformMatrix4x4, getNormalized(parentYAxis));
	setZAxis(parentTransformMatrix4x4, getNormalized(parentZAxis));
	setXAxis(childTransformMatrix4x4, getNormalized(childXAxis));
	setYAxis(childTransformMatrix4x4, getNormalized(childYAxis));
	setZAxis(childTransformMatrix4x4, getNormalized(childZAxis));

	const Matrix4x4 relativeTransformMatrix4x4 = childTransformMatrix4x4 * getInvertedCopy(parentTransformMatrix4x4);

	this->sceneGraphInstanceData.localPoseList[transformInstanceChild.index].position = getTranslation(relativeTransformMatrix4x4);
	this->sceneGraphInstanceData.localPoseList[transformInstanceChild.index].rotation = getRotationMatrix3x3Truncated(relativeTransformMatrix4x4);
	this->sceneGraphInstanceData.localPoseList[transformInstanceChild.index].scale = childScale;
	this->sceneGraphInstanceData.parentTransformInstanceList[transformInstanceChild.index] = transformInstanceParent;

	transform(parentTransformMatrix4x4, transformInstanceChild);
}

void SceneGraph::unlink(UnitId unitId)
{
	TransformInstance transformInstanceChild = makeTransformInstance(HashMapFn::get(unitIdToTransformInstanceMap, unitId, UINT32_MAX));
	RIO_ASSERT(transformInstanceChild.index < this->sceneGraphInstanceData.size, "Index out of bounds");

	if (!getIsValid(this->sceneGraphInstanceData.parentTransformInstanceList[transformInstanceChild.index]))
	{
		return;
	}

	if (!getIsValid(this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstanceChild.index]))
	{
		this->sceneGraphInstanceData.firstChildTransformInstanceList[this->sceneGraphInstanceData.parentTransformInstanceList[transformInstanceChild.index].index] = this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index];
	}
	else
	{
		this->sceneGraphInstanceData.nextSiblingTransformInstanceList[this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstanceChild.index].index] = this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index];
	}

	if (getIsValid(this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index]))
	{
		this->sceneGraphInstanceData.previousSiblingTransformInstanceList[this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index].index] = this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstanceChild.index];
	}

	this->sceneGraphInstanceData.parentTransformInstanceList[transformInstanceChild.index].index = UINT32_MAX;
	this->sceneGraphInstanceData.nextSiblingTransformInstanceList[transformInstanceChild.index].index = UINT32_MAX;
	this->sceneGraphInstanceData.previousSiblingTransformInstanceList[transformInstanceChild.index].index = UINT32_MAX;
}

void SceneGraph::clearChanged()
{
	for (uint32_t i = 0; i < this->sceneGraphInstanceData.size; ++i)
	{
		this->sceneGraphInstanceData.hasChangedList[i] = false;
	}
}

void SceneGraph::getAreChanged(Array<UnitId>& unitList, Array<Matrix4x4>& worldPoseList)
{
	for (uint32_t i = 0; i < this->sceneGraphInstanceData.size; ++i)
	{
		if (this->sceneGraphInstanceData.hasChangedList[i])
		{
			ArrayFn::pushBack(unitList, this->sceneGraphInstanceData.unitIdList[i]);
			ArrayFn::pushBack(worldPoseList, this->sceneGraphInstanceData.worldMatrix4x4List[i]);
		}
	}
}

void SceneGraph::setLocal(TransformInstance transformInstance)
{
	TransformInstance parentTransformInstance = this->sceneGraphInstanceData.parentTransformInstanceList[transformInstance.index];
	Matrix4x4 parentTransformMatrix4x4 = getIsValid(parentTransformInstance) ? this->sceneGraphInstanceData.worldMatrix4x4List[parentTransformInstance.index] : MATRIX4X4_IDENTITY;
	transform(parentTransformMatrix4x4, transformInstance);

	this->sceneGraphInstanceData.hasChangedList[transformInstance.index] = true;
}

void SceneGraph::transform(const Matrix4x4& parentMatrix4x4, TransformInstance transformInstance)
{
	this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index] = getLocalPose(this->sceneGraphInstanceData.unitIdList[transformInstance.index]) * parentMatrix4x4;

	TransformInstance childTransformInstance = this->sceneGraphInstanceData.firstChildTransformInstanceList[transformInstance.index];
	while (getIsValid(childTransformInstance) == true)
	{
		transform(this->sceneGraphInstanceData.worldMatrix4x4List[transformInstance.index], childTransformInstance);
		childTransformInstance = this->sceneGraphInstanceData.nextSiblingTransformInstanceList[childTransformInstance.index];
	}
}

void SceneGraph::grow()
{
	allocate(this->sceneGraphInstanceData.capacity * 2 + 1);
}

} // namespace Rio
