#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Types.h"

#include "World/Types.h"

namespace Rio
{

// A collection of nodes, possibly linked together to form a tree
struct SceneGraph
{
	struct Pose
	{
		Vector3 position;
		Matrix3x3 rotation;
		Vector3 scale;

		Pose& operator=(const Matrix4x4& matrix4x4);
	};

	struct SceneGraphInstanceData
	{
		SceneGraphInstanceData()
		{
		}

		uint32_t size = 0;
		uint32_t capacity = 0;
		void* buffer = nullptr;

		UnitId* unitIdList = nullptr;
		Matrix4x4* worldMatrix4x4List = nullptr;
		Pose* localPoseList = nullptr;
		TransformInstance* parentTransformInstanceList = nullptr;
		TransformInstance* firstChildTransformInstanceList = nullptr;
		TransformInstance* nextSiblingTransformInstanceList = nullptr;
		TransformInstance* previousSiblingTransformInstanceList = nullptr;
		bool* hasChangedList = nullptr;
	};

	uint32_t marker = SCENE_GRAPH_MARKER;
	Allocator* allocator = nullptr;
	UnitManager* unitManager = nullptr;
	SceneGraphInstanceData sceneGraphInstanceData;
	HashMap<UnitId, uint32_t> unitIdToTransformInstanceMap;

	SceneGraph(Allocator& a, UnitManager& unitManager);
	~SceneGraph();

	// Creates a new transform instance for unit <unitId>
	TransformInstance create(UnitId unitId, const Matrix4x4& pose);

	// Creates a new transform instance for unit <unitId>
	TransformInstance create(UnitId unitId, const Vector3& position, const Quaternion& rotation, const Vector3& scale);

	// Destroys the transform for the <unitId>
	// The transform is ignored
	void destroy(UnitId unitId, TransformInstance transformInstance);

	// Returns the transform instance of unit <unitId>
	TransformInstance getTransformInstanceByUnitId(UnitId unitId);

	// Returns whether the <unitId> has a transform
	bool has(UnitId unitId);

	// Sets the local position, rotation, scale or pose of the given <unitId>
	void setLocalPosition(UnitId unitId, const Vector3& position);
	void setLocalRotation(UnitId unitId, const Quaternion& rotation);
	void setLocalScale(UnitId unitId, const Vector3& scale);
	void setLocalPose(UnitId unitId, const Matrix4x4& pose);

	// Returns the local position, rotation or pose of the given <unitId>
	Vector3 getLocalPosition(UnitId unitId);
	Quaternion getLocalRotation(UnitId unitId);
	Vector3 getLocalScale(UnitId unitId);
	Matrix4x4 getLocalPose(UnitId unitId);

	// Returns the world position, rotation or pose of the given <unitId>
	Vector3 getWorldPosition(UnitId unitId);
	Quaternion getWorldRotation(UnitId unitId);
	Matrix4x4 getWorldPose(UnitId unitId);

	void setWorldPose(TransformInstance transformInstance, const Matrix4x4& pose);

	// Returns the number of nodes in the graph
	uint32_t getNodeListCount() const;

	// Links the unit <childUnitId> to the unit <parentUnitId>
	void link(UnitId childUnitId, UnitId parentUnitId);

	// Unlinks the <unitId> from its parent if it has any
	// After unlinking, the unit's local pose is set to its previous world pose
	void unlink(UnitId unitId);

	void clearChanged();
	void getAreChanged(Array<UnitId>& unitList, Array<Matrix4x4>& worldPoseList);
	void setLocal(TransformInstance transformInstance);
	void transform(const Matrix4x4& parentMatrix4x4, TransformInstance transformInstance);
	void grow();
	void allocate(uint32_t instanceDataSize);
	TransformInstance makeTransformInstance(uint32_t index);
	void unitDestroyedCallback(UnitId unitId);
};

} // namespace Rio
