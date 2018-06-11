#pragma once

#include "Config.h"

#include "Core/Containers/EventStream.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Script/Types.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#include "Resource/Types.h"

#include "World/Gui/Gui.h"
#include "World/Types.h"

namespace Rio
{

struct World
{
	struct Camera
	{
		UnitId unitId;

		ProjectionType::Enum projectionType = ProjectionType::COUNT;

		Frustum frustum;

		float fov = 0.0f;
		float aspect = 0.0f;
		float nearRange = 0.0f;
		float farRange = 0.0f;

		// For orthographic projection only
		float halfSize = 0.0f;

		uint16_t viewX = 0;
		uint16_t viewY = 0;
		uint16_t viewWidth = 0;
		uint16_t viewHeight = 0;
	};

	uint32_t marker = WORLD_MARKER;
	Allocator* allocator = nullptr;

	ResourceManager* resourceManager = nullptr;
	
	ShaderManager* shaderManager = nullptr;
	MaterialManager* materialManager = nullptr;

#if AMSTEL_ENGINE_SCRIPT_LUA
	LuaEnvironment* luaEnvironment = nullptr;
#endif // AMSTEL_ENGINE_SCRIPT_LUA
	
	UnitManager* unitManager = nullptr;

	DebugLine* debugLine = nullptr;

	SceneGraph* sceneGraph = nullptr;
	RenderWorld* renderWorld = nullptr;

#if AMSTEL_ENGINE_PHYSICS
	PhysicsWorld* physicsWorld = nullptr;
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SOUND
	SoundWorld* soundWorld = nullptr;
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_SCRIPT_LUA
	ScriptWorld* scriptWorld = nullptr;
#endif // AMSTEL_ENGINE_SCRIPT_LUA
	
	AnimationStateMachine* animationStateMachine = nullptr;

	Array<UnitId> unitIdList;
	Array<Level*> levelList;
	Array<Camera> cameraList;
	HashMap<UnitId, uint32_t> unitIdToCameraInstanceMap;

	EventStream eventStream;

	GuiBuffer guiBuffer;
	Array<Gui*> guiList;

	CameraInstance cameraMakeCameraInstance(uint32_t index) 
	{ 
		CameraInstance cameraInstance = { index };
		return cameraInstance;
	}

#if AMSTEL_ENGINE_SCRIPT_LUA
	World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager, LuaEnvironment& luaEnvironment);
#else
	World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager);
#endif // AMSTEL_ENGINE_SCRIPT_LUA
	~World();

	// Spawns a new instance of the unit <name> at the given <position> and <rotation>
	UnitId spawnUnit(StringId64 name, const Vector3& position = VECTOR3_ZERO, const Quaternion& rotation = QUATERNION_IDENTITY);

	// Creates a new empty unit and returns its id
	UnitId createNewEmptyUnit();

	// Destroys the unit with the given <unitId>
	void destroyUnit(UnitId unitId);

	// Returns the number of units in the world
	uint32_t getUnitListCount() const;

	// Returns all the the units in the world
	void getUnitIdList(Array<UnitId>& unitIdList) const;

	// Creates a new camera
	CameraInstance cameraCreate(UnitId unitId, const CameraDesc& cameraDesc, const Matrix4x4& transformMatrix4x4);

	// Destroys the camera <cameraInstance>
	void cameraDestroy(UnitId unitId, CameraInstance cameraInstance);

	// Returns the camera owned by unit <unitId>
	CameraInstance getCameraInstanceByUnitId(UnitId unitId);

	// Sets the projection type of the camera
	void setCameraProjectionType(UnitId unitId, ProjectionType::Enum type);

	// Returns the projection type of the camera
	ProjectionType::Enum getCameraProjectionType(UnitId unitId);

	// Returns the projection matrix of the camera
	Matrix4x4 getCameraProjectionMatrix(UnitId unitId);

	// Returns the view matrix of the camera
	Matrix4x4 getCameraViewMatrix(UnitId unitId);

	// Returns the field-of-view of the camera in degrees
	float getCameraFov(UnitId unitId);

	// Sets the field-of-view of the camera in degrees
	void setCameraFov(UnitId unitId, float fov);

	// Sets the aspect ratio of the camera (Perspective projection only)
	void setCameraAspect(UnitId unitId, float aspect);

	// Returns the near clip distance of the camera
	float getCameraNearClipDistance(UnitId unitId);

	// Sets the near clip distance of the camera
	void setCameraNearClipDistance(UnitId unitId, float nearRange);

	// Returns the far clip distance of the camera
	float getCameraFarClipDistance(UnitId unitId);

	// Sets the far clip distance of the camera
	void setCameraFarClipDistance(UnitId unitId, float farRange);

	// Sets the vertical <halfSize> of the orthographic view volume
	// The horizontal size is proportional to the viewport's aspect ratio
	void setCameraOrthographicSize(UnitId unitId, float halfSize);

	// Sets the coordinates for the camera viewport in pixels
	void setCameraViewportMetrics(UnitId unitId, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

	// Returns <position> from screen-space to world-space coordinates
	Vector3 getCameraScreenToWorldPosition(UnitId unitId, const Vector3& position);

	// Returns <position> from world-space to screen-space coordinates
	Vector3 getCameraWorldToScreenPosition(UnitId unitId, const Vector3& position);

	// Update all animations with <dt>
	void updateAnimationList(float dt);

	// Update scene with <dt>
	void updateScene(float dt);

	// Updates all units and sub-systems with the given <dt> delta time
	void update(float dt);

	// Renders the world using <viewMatrix4x4> and <projectionMatrix4x4>
	void render(const Matrix4x4& viewMatrix4x4, const Matrix4x4& projectionMatrix4x4);

#if AMSTEL_ENGINE_SOUND

	SoundInstanceId playSound(const SoundResource& soundResource, bool loop = false, float volume = 1.0f, const Vector3& position = VECTOR3_ZERO, float range = 50.0f);

	// Plays the sound with the given <name> at the given <position>, with the given <volume> and <range>
	// <loop> controls whether the sound must loop or not
	SoundInstanceId playSound(StringId64 name, const bool loop, const float volume, const Vector3& position, const float range);

	// Stops the sound with the given <soundInstanceId>
	void stopSound(SoundInstanceId soundInstanceId);

	// Links the sound <soundInstanceId> to the <nodeIndex> of the given <unitId>
	// After this call, the sound <soundInstanceId> will follow the unit <unitId>
	void linkSound(SoundInstanceId soundInstanceId, UnitId unitId, int32_t nodeIndex);

	// Sets the <pose> of the listener
	void setSoundListenerPose(const Matrix4x4& pose);

	// Sets the <position> of the sound <soundInstanceId>
	void setSoundPosition(SoundInstanceId soundInstanceId, const Vector3& position);

	// Sets the <range> of the sound <soundInstanceId>
	void setSoundRange(SoundInstanceId soundInstanceId, float range);

	// Sets the <volume> of the sound <soundInstanceId>
	void setSoundVolume(SoundInstanceId soundInstanceId, float volume);

#endif // AMSTEL_ENGINE_SOUND

	// Creates a new DebugLine
	// <enableDepthTest> controls whether to enable depth test when rendering the lines
	DebugLine* createDebugLine(bool enableDepthTest);

	// Destroys the debug <debugLine>
	void destroyDebugLine(DebugLine& debugLine);

	// Creates a new screen space Gui
	Gui* createScreenGui();

	// Destroys the <gui>
	void destroyScreenGui(Gui& gui);

	// Loads the level <name> into the world
	Level* loadLevel(StringId64 name, const Vector3& position, const Quaternion& rotation);

	void postUnitSpawnedEvent(UnitId unitId);
	void postUnitDestroyedEvent(UnitId unitId);
	void postLevelLoadedEvent();
};

void spawnUnitList(World& world, const UnitResource& unitResource, const Vector3& position, const Quaternion& rotation, const UnitId* unitLookupMap);

} // namespace Rio
