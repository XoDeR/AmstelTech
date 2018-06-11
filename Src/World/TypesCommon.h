#pragma once

#include "Config.h"

#include "Core/Functional.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "World/TypesWorld.h" // UnitId

namespace Rio
{

	struct AnimationStateMachine;
	struct DebugLine;
	struct Gui;
	struct Level;
	struct Material;
	struct MaterialManager;
	struct RenderWorld;
	struct SceneGraph;
	struct ShaderManager;
	struct UnitManager;
	struct World;

	// Murmur hash 3 unsigned 32-bit

#define DEBUG_LINE_MARKER 0xdd63448c
#define DEBUG_GUI_MARKER 0xf41b5f74
#define LEVEL_MARKER 0x8ef9a681
#define RENDER_WORLD_MARKER 0xd24298b1
#define RESOURCE_PACKAGE_MARKER 0xf2b1d443
#define SCENE_GRAPH_MARKER 0xa806010f
#define WORLD_MARKER 0xffcdd512
#define ANIMATION_STATE_MACHINE_MARKER 0x9cc5e6ac

	static const StringId32 COMPONENT_TYPE_CAMERA = StringId32("camera");
	static const StringId32 COMPONENT_TYPE_LIGHT = StringId32("light");
	static const StringId32 COMPONENT_TYPE_MESH_RENDERER = StringId32("meshRenderer");
	static const StringId32 COMPONENT_TYPE_SPRITE_RENDERER = StringId32("spriteRenderer");
	static const StringId32 COMPONENT_TYPE_TRANSFORM = StringId32("transform");
	static const StringId32 COMPONENT_TYPE_ANIMATION_STATE_MACHINE = StringId32("animationStateMachine");

	struct ProjectionType
	{
		enum Enum
		{
			ORTHOGRAPHIC,
			PERSPECTIVE,

			COUNT
		};
	};

	struct LightType
	{
		enum Enum
		{
			DIRECTIONAL,
			OMNI,
			SPOT,

			COUNT
		};
	};

	struct EventType
	{
		enum Enum
		{
			UNIT_SPAWNED,
			UNIT_DESTROYED,

			LEVEL_LOADED,

#if AMSTEL_ENGINE_PHYSICS
			PHYSICS_COLLISION,
			PHYSICS_TRIGGER,
			PHYSICS_TRANSFORM,
#endif // AMSTEL_ENGINE_PHYSICS

			COUNT
		};
	};

	// TransformInstance
	struct TransformInstance
	{
		uint32_t index = UINT32_MAX;
	};

	inline bool getIsValid(TransformInstance transformInstance)
	{
		return transformInstance.index != UINT32_MAX;
	}

	// CameraInstance
	struct CameraInstance
	{
		uint32_t index = UINT32_MAX;
	};

	inline bool getIsValid(CameraInstance cameraInstance)
	{
		return cameraInstance.index != UINT32_MAX;
	}

	// MeshInstance
	struct MeshInstance
	{
		uint32_t index = UINT32_MAX;
	};

	inline bool getIsValid(MeshInstance meshInstance)
	{
		return meshInstance.index != UINT32_MAX;
	}

	// SpriteInstance
	struct SpriteInstance
	{
		uint32_t index = UINT32_MAX;
	};

	inline bool getIsValid(SpriteInstance spriteInstance)
	{
		return spriteInstance.index != UINT32_MAX;
	}

	// LightInstance
	struct LightInstance
	{
		uint32_t index = UINT32_MAX;
	};

	inline bool getIsValid(LightInstance lightInstance)
	{
		return lightInstance.index != UINT32_MAX;
	}

	struct MeshRendererDesc
	{
		StringId64 meshResource; // Name of mesh resource
		StringId64 materialResource; // Name of material resource
		StringId32 geometryName; // Name of geometry inside mesh resource
		bool visible = true; // Whether mesh is visible
		char padding00[3] = { 0, 0, 0 };
	};

	struct SpriteRendererDesc
	{
		StringId64 spriteResource; // Name of sprite resource
		StringId64 materialResource; // Name of material resource
		uint32_t layer = 0; // Sort layer
		uint32_t depth = 0; // Depth in layer
		bool visible = true; // Whether sprite is visible
		char padding00[3] = { 0, 0, 0 };
		char padding01[4] = { 0, 0, 0, 0 };
	};

	struct AnimationStateMachineDesc
	{
		StringId64 stateMachineResource; // Name of stateMachineResource resource
	};

	struct LightDesc
	{
		uint32_t type = UINT32_MAX; // LightType::Enum
		float range = 0.0f; // In meters
		float intensity = 0.0f;
		float spotAngle = 0.0f; // In radians
		Vector3 color;  // Color of the light
	};

	struct TransformDesc
	{
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
	};

	struct CameraDesc
	{
		uint32_t type = UINT32_MAX; // ProjectionType::Enum
		float fov = 0.0f; // Vertical FOV
		float nearRange = 0.0f; // Near clipping plane distance
		float farRange = 0.0f;  // Far clipping plane distance
	};

	struct UnitSpawnedEvent
	{
		UnitId unitId; // The unit spawned
	};

	struct UnitDestroyedEvent
	{
		UnitId unitId; // The unit destroyed
	};

	struct LevelLoadedEvent
	{
	};

} // namespace Rio