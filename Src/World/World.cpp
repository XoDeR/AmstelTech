#include "World/World.h"

#include "Core/Containers/HashMap.h"
#include "Core/Error/Error.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/TempAllocator.h"

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Script/LuaEnvironment.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#include "Resource/ResourceManager.h"
#include "Resource/UnitResource.h"

#include "World/Sprite/AnimationStateMachine.h"
#include "World/Renderer/DebugLine.h"
#include "World/Gui/Gui.h"
#include "World/Level.h"
#include "World/Renderer/RenderWorld.h"
#include "World/Renderer/SceneGraph.h"
#include "World/UnitManager.h"

#if AMSTEL_ENGINE_SOUND
#include "World/Audio/SoundWorld.h"
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
#include "World/Physics/PhysicsWorld.h"
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "World/Script/ScriptWorld.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

namespace Rio
{

#if AMSTEL_ENGINE_SCRIPT_LUA
World::World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager, LuaEnvironment& luaEnvironment)
	: allocator(&a)
	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, luaEnvironment(&luaEnvironment)
	, unitManager(&unitManager)

	, unitIdList(a)
	, levelList(a)
	, cameraList(a)
	, unitIdToCameraInstanceMap(a)
	, eventStream(a)

	, guiBuffer(shaderManager)
	, guiList(a)
{
	debugLine = createDebugLine(true);

	sceneGraph = RIO_NEW(*allocator, SceneGraph)(*allocator, unitManager);
	renderWorld = RIO_NEW(*allocator, RenderWorld)(*allocator, resourceManager, shaderManager, materialManager, unitManager);

#if AMSTEL_ENGINE_PHYSICS
	physicsWorld = RIO_NEW(*allocator, PhysicsWorld)(*allocator, resourceManager, unitManager, *debugLine);
#endif // AMSTEL_ENGINE_PHYSICS
	
#if AMSTEL_ENGINE_SOUND
	soundWorld = RIO_NEW(*allocator, SoundWorld)(*allocator);
#endif // AMSTEL_ENGINE_SOUND
	
	scriptWorld = RIO_NEW(*allocator, ScriptWorld)(*allocator, unitManager, resourceManager, luaEnvironment, *this);

	this->animationStateMachine = RIO_NEW(*allocator, AnimationStateMachine)(*allocator, resourceManager, unitManager);

	this->guiBuffer.create();
}

#else

World::World(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager)
	: allocator(&a)

	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, unitManager(&unitManager)

	, unitIdList(a)
	, levelList(a)
	, cameraList(a)
	, unitIdToCameraInstanceMap(a)
	, eventStream(a)

	, guiBuffer(shaderManager)
	, guiList(a)
{
	debugLine = createDebugLine(true);

	sceneGraph = RIO_NEW(*allocator, SceneGraph)(*allocator, unitManager);
	renderWorld = RIO_NEW(*allocator, RenderWorld)(*allocator, resourceManager, shaderManager, materialManager, unitManager);

#if AMSTEL_ENGINE_PHYSICS
	physicsWorld = RIO_NEW(*allocator, PhysicsWorld)(*allocator, resourceManager, unitManager, *debugLine);
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SOUND
	soundWorld = RIO_NEW(*allocator, SoundWorld)(*allocator);
#endif // AMSTEL_ENGINE_SOUND

	this->animationStateMachine = RIO_NEW(*allocator, AnimationStateMachine)(*allocator, resourceManager, unitManager);

	this->guiBuffer.create();
}

#endif // AMSTEL_ENGINE_SCRIPT_LUA

World::~World()
{
	for (uint32_t i = 0; i < ArrayFn::getCount(this->levelList); ++i)
	{
		RIO_DELETE(*allocator, this->levelList[i]);
	}

	for (uint32_t i = 0; i < ArrayFn::getCount(this->unitIdList); ++i)
	{
		unitManager->destroy(this->unitIdList[i]);
	}

	RIO_DELETE(*allocator, this->animationStateMachine);

#if AMSTEL_ENGINE_SCRIPT_LUA
	RIO_DELETE(*allocator, scriptWorld);
#endif // AMSTEL_ENGINE_SCRIPT_LUA	
	
#if AMSTEL_ENGINE_SOUND
	RIO_DELETE(*allocator, soundWorld);
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
	RIO_DELETE(*allocator, physicsWorld);
#endif // AMSTEL_ENGINE_PHYSICS
	
	RIO_DELETE(*allocator, renderWorld);
	RIO_DELETE(*allocator, sceneGraph);

	destroyDebugLine(*debugLine);

	marker = 0;
}

UnitId World::spawnUnit(StringId64 name, const Vector3& position, const Quaternion& rotation)
{
	const UnitResource* unitResource = (const UnitResource*)resourceManager->getResourceData(RESOURCE_TYPE_UNIT, name);
	UnitId unitId = unitManager->createNewUnit();
	spawnUnitList(*this, *unitResource, position, rotation, &unitId);
	return unitId;
}

UnitId World::createNewEmptyUnit()
{
	UnitId unitId = unitManager->createNewUnit();
	ArrayFn::pushBack(this->unitIdList, unitId);
	postUnitSpawnedEvent(unitId);
	return unitId;
}

void World::destroyUnit(UnitId unitId)
{
	unitManager->destroy(unitId);
	for (uint32_t i = 0, n = ArrayFn::getCount(this->unitIdList); i < n; ++i)
	{
		if (this->unitIdList[i] == unitId)
		{
			this->unitIdList[i] = this->unitIdList[n - 1];
			ArrayFn::popBack(this->unitIdList);
			break;
		}
	}
	postUnitDestroyedEvent(unitId);
}

uint32_t World::getUnitListCount() const
{
	return ArrayFn::getCount(this->unitIdList);
}

void World::getUnitIdList(Array<UnitId>& unitIdList) const
{
	ArrayFn::reserve(unitIdList, ArrayFn::getCount(this->unitIdList));
	ArrayFn::push(unitIdList, ArrayFn::begin(this->unitIdList), ArrayFn::getCount(this->unitIdList));
}

void World::updateAnimationList(float dt)
{
	this->animationStateMachine->update(dt);
}

void World::updateScene(float dt)
{
	// Process animation events
	{
		EventStream& eventStream = this->animationStateMachine->eventStream;
		const uint32_t size = ArrayFn::getCount(eventStream);
		uint32_t bytesRead = 0;
		while (bytesRead < size)
		{
			const EventHeader* eventHeader = (EventHeader*)&eventStream[bytesRead];
			const char* eventStreamData = (char*)&eventHeader[1];

			bytesRead += sizeof(*eventHeader) + eventHeader->size;

			switch (eventHeader->type)
			{
			case 0:
				{
					const SpriteFrameChangeEvent& spriteFrameChangeEvent = *(SpriteFrameChangeEvent*)eventStreamData;
					renderWorld->spriteSetFrame(spriteFrameChangeEvent.unitId, spriteFrameChangeEvent.frameId);
				}
				break;

			default:
				RIO_FATAL("Unknown event type");
				break;
			}
		}
		ArrayFn::clear(eventStream);
	}

	TempAllocator4096 tempAllocator4096;
	Array<UnitId> changedUnitList(tempAllocator4096);
	Array<Matrix4x4> changedWorldMatrix4x4List(tempAllocator4096);

	this->sceneGraph->getAreChanged(changedUnitList, changedWorldMatrix4x4List);

#if AMSTEL_ENGINE_PHYSICS

	physicsWorld->updateActorWorldPoseList(ArrayFn::begin(changedUnitList)
		, ArrayFn::end(changedUnitList)
		, ArrayFn::begin(changedWorldMatrix4x4List)
		);

	physicsWorld->update(dt);

	// Process physics events
	{
		EventStream& eventStream = physicsWorld->getEventStream();
		const uint32_t size = ArrayFn::getCount(eventStream);
		uint32_t bytesRead = 0;
		while (bytesRead < size)
		{
			const EventHeader* eventHeader = (EventHeader*)&eventStream[bytesRead];
			const char* eventStreamData = (char*)&eventHeader[1];

			bytesRead += sizeof(*eventHeader) + eventHeader->size;

			switch (eventHeader->type)
			{
			case EventType::PHYSICS_TRANSFORM:
			{
				const PhysicsTransformEvent& physicsTransformEvent = *(PhysicsTransformEvent*)eventStreamData;
				const TransformInstance transformInstance = this->sceneGraph->getTransformInstanceByUnitId(physicsTransformEvent.unitId);
				const Matrix4x4 pose = createMatrix4x4(physicsTransformEvent.rotation, ptev.position);
				this->sceneGraph->setWorldPose(transformInstance, pose);
			}
			break;

			case EventType::PHYSICS_COLLISION:
			{
				const PhysicsCollisionEvent& physicsCollisionEvent = *(PhysicsCollisionEvent*)eventStreamData;
#if AMSTEL_ENGINE_SCRIPT_LUA
				ScriptWorldFn::collision(*scriptWorld, physicsCollisionEvent);
#endif // AMSTEL_ENGINE_SCRIPT_LUA
			}
			break;

			case EventType::PHYSICS_TRIGGER:
				break;

			default:
				RIO_FATAL("Unknown event type");
				break;
			}
		}
		ArrayFn::clear(eventStream);
	}

#endif // AMSTEL_ENGINE_PHYSICS

	ArrayFn::clear(changedUnitList);
	ArrayFn::clear(changedWorldMatrix4x4List);
	this->sceneGraph->getAreChanged(changedUnitList, changedWorldMatrix4x4List);
	this->sceneGraph->clearChanged();

	renderWorld->updateUnitTransformList(ArrayFn::begin(changedUnitList)
		, ArrayFn::end(changedUnitList)
		, ArrayFn::begin(changedWorldMatrix4x4List)
		);

#if AMSTEL_ENGINE_SOUND
	soundWorld->update();
#endif // AMSTEL_ENGINE_SOUND

	this->guiBuffer.reset();

	ArrayFn::clear(this->eventStream);

#if AMSTEL_ENGINE_SCRIPT_LUA
	ScriptWorldFn::update(*scriptWorld, dt);
#endif // AMSTEL_ENGINE_SCRIPT_LUA
}

void World::update(float dt)
{
	updateAnimationList(dt);
	updateScene(dt);
}

void World::render(const Matrix4x4& viewMatrix4x4, const Matrix4x4& projectionMatrix4x4)
{
	renderWorld->render(viewMatrix4x4, projectionMatrix4x4);

#if AMSTEL_ENGINE_PHYSICS
	physicsWorld->debugDraw();
#endif // AMSTEL_ENGINE_PHYSICS

	renderWorld->debugDraw(*debugLine);

	debugLine->submit();
	debugLine->reset();
}

CameraInstance World::cameraCreate(UnitId unitId, const CameraDesc& cameraDesc, const Matrix4x4& /*transformMatrix4x4*/)
{
	Camera camera;
	camera.unitId = unitId;
	camera.projectionType = (ProjectionType::Enum)cameraDesc.type;
	camera.fov = cameraDesc.fov;
	camera.nearRange = cameraDesc.nearRange;
	camera.farRange = cameraDesc.farRange;

	const uint32_t lastIndex = ArrayFn::getCount(this->cameraList);
	ArrayFn::pushBack(this->cameraList, camera);

	HashMapFn::set(this->unitIdToCameraInstanceMap, unitId, lastIndex);
	return cameraMakeCameraInstance(lastIndex);
}

void World::cameraDestroy(UnitId unitId, CameraInstance cameraInstance)
{
	const uint32_t lastIndex = ArrayFn::getCount(this->cameraList) - 1;
	const UnitId unitIdLocal = this->cameraList[cameraInstance.index].unitId;
	const UnitId lastUnitId = this->cameraList[lastIndex].unitId;

	this->cameraList[cameraInstance.index] = this->cameraList[lastIndex];

	HashMapFn::set(this->unitIdToCameraInstanceMap, lastUnitId, cameraInstance.index);
	HashMapFn::remove(this->unitIdToCameraInstanceMap, unitIdLocal);
}

CameraInstance World::getCameraInstanceByUnitId(UnitId unitId)
{
	return cameraMakeCameraInstance(HashMapFn::get(this->unitIdToCameraInstanceMap, unitId, UINT32_MAX));
}

void World::setCameraProjectionType(UnitId unitId, ProjectionType::Enum type)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].projectionType = type;
}

ProjectionType::Enum World::getCameraProjectionType(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	return this->cameraList[cameraInstance.index].projectionType;
}

Matrix4x4 World::getCameraProjectionMatrix(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	Camera& camera = this->cameraList[cameraInstance.index];
	Matrix4x4 projectionMatrix4x4;

	switch (camera.projectionType)
	{
	case ProjectionType::ORTHOGRAPHIC:
		setToOrthographic(projectionMatrix4x4
			, -camera.halfSize * camera.aspect
			, camera.halfSize * camera.aspect
			, -camera.halfSize
			, camera.halfSize
			, camera.nearRange
			, camera.farRange
			);
		break;

	case ProjectionType::PERSPECTIVE:
		setToPerspective(projectionMatrix4x4
			, camera.fov
			, camera.aspect
			, camera.nearRange
			, camera.farRange
			);
		break;

	default:
		RIO_FATAL("Unknown projection type");
		break;
	}

	return projectionMatrix4x4;
}

Matrix4x4 World::getCameraViewMatrix(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	Matrix4x4 viewMatrix4x4 = this->sceneGraph->getWorldPose(this->cameraList[cameraInstance.index].unitId);
	getInverted(viewMatrix4x4);
	return viewMatrix4x4;
}

float World::getCameraFov(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	return this->cameraList[cameraInstance.index].fov;
}

void World::setCameraFov(UnitId unitId, float fov)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].fov = fov;
}

void World::setCameraAspect(UnitId unitId, float aspect)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].aspect = aspect;
}

float World::getCameraNearClipDistance(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	return this->cameraList[cameraInstance.index].nearRange;
}

void World::setCameraNearClipDistance(UnitId unitId, float nearRange)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].nearRange = nearRange;
}

float World::getCameraFarClipDistance(UnitId unitId)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	return this->cameraList[cameraInstance.index].farRange;
}

void World::setCameraFarClipDistance(UnitId unitId, float farRange)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].farRange = farRange;
}

void World::setCameraOrthographicSize(UnitId unitId, float halfSize)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].halfSize = halfSize;
}

void World::setCameraViewportMetrics(UnitId unitId, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	this->cameraList[cameraInstance.index].viewX = x;
	this->cameraList[cameraInstance.index].viewY = y;
	this->cameraList[cameraInstance.index].viewWidth = width;
	this->cameraList[cameraInstance.index].viewHeight = height;
}

Vector3 World::getCameraScreenToWorldPosition(UnitId unitId, const Vector3& position)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	const Camera& camera = this->cameraList[cameraInstance.index];

	Matrix4x4 projectionMatrix4x4 = getCameraProjectionMatrix(unitId);
	Matrix4x4 worldInvertedMatrix4x4 = this->sceneGraph->getWorldPose(camera.unitId);
	getInverted(worldInvertedMatrix4x4);
	Matrix4x4 modelViewProjectionMatrix4x4 = worldInvertedMatrix4x4 * projectionMatrix4x4;
	getInverted(modelViewProjectionMatrix4x4);

	Vector4 ndc;
	ndc.x = (2.0f * (position.x - 0.0f)) / camera.viewWidth - 1.0f;
	ndc.y = (2.0f * (camera.viewHeight - position.y)) / camera.viewHeight - 1.0f;
	ndc.z = (2.0f * position.z) - 1.0f;
	ndc.w = 1.0f;

	Vector4 tempVector4 = ndc * modelViewProjectionMatrix4x4;
	tempVector4 *= 1.0f / tempVector4.w;

	return createVector3(tempVector4.x, tempVector4.y, tempVector4.z);
}

Vector3 World::getCameraWorldToScreenPosition(UnitId unitId, const Vector3& position)
{
	CameraInstance cameraInstance = getCameraInstanceByUnitId(unitId);
	const Camera& camera = this->cameraList[cameraInstance.index];

	Matrix4x4 projectionMatrix4x4 = getCameraProjectionMatrix(unitId);
	Matrix4x4 worldInvertedMatrix4x4 = this->sceneGraph->getWorldPose(camera.unitId);
	getInverted(worldInvertedMatrix4x4);

	Vector4 xyzw;
	xyzw.x = position.x;
	xyzw.y = position.y;
	xyzw.z = position.z;
	xyzw.w = 1.0f;

	Vector4 clip = xyzw * (worldInvertedMatrix4x4 * projectionMatrix4x4);

	Vector4 ndc;
	ndc.x = clip.x / clip.w;
	ndc.y = clip.y / clip.w;

	Vector3 screen;
	screen.x = (camera.viewX + camera.viewWidth  * (ndc.x + 1.0f)) / 2.0f;
	screen.y = (camera.viewY + camera.viewHeight * (1.0f - ndc.y)) / 2.0f;
	screen.z = 0.0f;

	return screen;
}

#if AMSTEL_ENGINE_SOUND

SoundInstanceId World::playSound(const SoundResource& soundResource, const bool loop, const float volume, const Vector3& position, const float range)
{
	return soundWorld->play(soundResource, loop, volume, range, position);
}

SoundInstanceId World::playSound(StringId64 name, const bool loop, const float volume, const Vector3& position, const float range)
{
	const SoundResource* soundResource = (const SoundResource*)resourceManager->getResourceData(RESOURCE_TYPE_SOUND, name);
	return playSound(*soundResource, loop, volume, position, range);
}

void World::stopSound(SoundInstanceId soundInstanceId)
{
	soundWorld->stop(soundInstanceId);
}

void World::linkSound(SoundInstanceId /*soundInstanceId*/, UnitId /*unitId*/, int32_t /*nodeIndex*/)
{
	RIO_FATAL("Not implemented yet");
}

void World::setSoundListenerPose(const Matrix4x4& pose)
{
	soundWorld->setListenerPose(pose);
}

void World::setSoundPosition(SoundInstanceId soundInstanceId, const Vector3& position)
{
	soundWorld->setSoundPositionList(1, &soundInstanceId, &position);
}

void World::setSoundRange(SoundInstanceId soundInstanceId, float range)
{
	soundWorld->setSoundRangeList(1, &soundInstanceId, &range);
}

void World::setSoundVolume(SoundInstanceId soundInstanceId, float volume)
{
	soundWorld->setSoundVolumeList(1, &soundInstanceId, &volume);
}

#endif // AMSTEL_ENGINE_SOUND

DebugLine* World::createDebugLine(bool enableDepthTest)
{
	return RIO_NEW(*allocator, DebugLine)(*shaderManager, enableDepthTest);
}

void World::destroyDebugLine(DebugLine& debugLine)
{
	RIO_DELETE(*allocator, &debugLine);
}

Gui* World::createScreenGui()
{
	Gui* gui = RIO_NEW(*allocator, Gui)(this->guiBuffer, *resourceManager
		, *shaderManager
		, *materialManager
		);
	ArrayFn::pushBack(guiList, gui);
	return gui;
}

void World::destroyScreenGui(Gui& gui)
{
	for (uint32_t i = 0, n = ArrayFn::getCount(guiList); i < n; ++i)
	{
		if (guiList[i] == &gui)
		{
			RIO_DELETE(*allocator, &gui);
			guiList[i] = guiList[n - 1];
			ArrayFn::popBack(guiList);
			return;
		}
	}

	RIO_FATAL("Gui not found");
}

Level* World::loadLevel(StringId64 name, const Vector3& position, const Quaternion& rotation)
{
	const LevelResource* levelResource = (const LevelResource*)resourceManager->getResourceData(RESOURCE_TYPE_LEVEL, name);

	Level* level = RIO_NEW(*allocator, Level)(*allocator, *unitManager, *this, *levelResource);
	level->load(position, rotation);

	ArrayFn::pushBack(this->levelList, level);
	postLevelLoadedEvent();

	return level;
}

void World::postUnitSpawnedEvent(UnitId unitId)
{
	UnitSpawnedEvent unitSpawnedEvent;
	unitSpawnedEvent.unit = unitId;
	EventStreamFn::write(this->eventStream, EventType::UNIT_SPAWNED, unitSpawnedEvent);
}

void World::postUnitDestroyedEvent(UnitId unitId)
{
	UnitDestroyedEvent unitDestroyedEvent;
	unitDestroyedEvent.unit = unitId;
	EventStreamFn::write(this->eventStream, EventType::UNIT_DESTROYED, unitDestroyedEvent);
}

void World::postLevelLoadedEvent()
{
	LevelLoadedEvent levelLoadedEvent;
	EventStreamFn::write(this->eventStream, EventType::LEVEL_LOADED, levelLoadedEvent);
}

void spawnUnitList(World& world, const UnitResource& unitResource, const Vector3& position, const Quaternion& rotation, const UnitId* unitLookupMap)
{
	const ComponentData* componentData = (ComponentData*)(&unitResource + 1);
	for (uint32_t componentCounter = 0; componentCounter < unitResource.componentTypeListCount; ++componentCounter, componentData = (ComponentData*)((char*)componentData + componentData->size + sizeof(*componentData)))
	{
		const uint32_t* unitIndexList = (const uint32_t*)(componentData + 1);
		const char* componentDataBufferBegin = (const char*)(unitIndexList + componentData->instanceListCount);

		// unit may have multiple components of the same type -- componentData->instanceListCount
		if (componentData->type == COMPONENT_TYPE_TRANSFORM)
		{
			const TransformDesc* transformDescList = (const TransformDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++transformDescList)
			{
				Matrix4x4 unitMatrix4x4 = createMatrix4x4(rotation, position);
				Matrix4x4 transformMatrix4x4 = createMatrix4x4(transformDescList->rotation, transformDescList->position);
				world.sceneGraph->create(unitLookupMap[unitIndexList[i]], transformMatrix4x4 * unitMatrix4x4);
			}
		}
		else if (componentData->type == COMPONENT_TYPE_CAMERA)
		{
			const CameraDesc* cameraDesc = (const CameraDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++cameraDesc)
			{
				world.cameraCreate(unitLookupMap[unitIndexList[i]], *cameraDesc, MATRIX4X4_IDENTITY);
			}
		}

#if	AMSTEL_ENGINE_PHYSICS
		else if (componentData->type == COMPONENT_TYPE_COLLIDER)
		{
			const ColliderDesc* colliderDesc = (const ColliderDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i)
			{
				world.physicsWorld->colliderCreate(unitLookupMap[unitIndexList[i]], colliderDesc);
				colliderDesc = (ColliderDesc*)((char*)(colliderDesc + 1) + colliderDesc->size);
			}
		}
		else if (componentData->type == COMPONENT_TYPE_ACTOR)
		{
			const ActorResource* actorResource = (const ActorResource*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++actorResource)
			{
				Matrix4x4 transformMatrix = world.sceneGraph->getWorldPose(unitLookupMap[unitIndexList[i]]);
				world.physicsWorld->actorCreate(unitLookupMap[unitIndexList[i]], actorResource, transformMatrix);
			}
		}
#endif	AMSTEL_ENGINE_PHYSICS

		else if (componentData->type == COMPONENT_TYPE_MESH_RENDERER)
		{
			const MeshRendererDesc* meshRendererDesc = (const MeshRendererDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++meshRendererDesc)
			{
				Matrix4x4 transformMatrix = world.sceneGraph->getWorldPose(unitLookupMap[unitIndexList[i]]);
				world.renderWorld->meshCreate(unitLookupMap[unitIndexList[i]], *meshRendererDesc, transformMatrix);
			}
		}
		else if (componentData->type == COMPONENT_TYPE_SPRITE_RENDERER)
		{
			const SpriteRendererDesc* spriteRendererDesc = (const SpriteRendererDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++spriteRendererDesc)
			{
				Matrix4x4 transformMatrix = world.sceneGraph->getWorldPose(unitLookupMap[unitIndexList[i]]);
				world.renderWorld->spriteCreate(unitLookupMap[unitIndexList[i]], *spriteRendererDesc, transformMatrix);
			}
		}
		else if (componentData->type == COMPONENT_TYPE_LIGHT)
		{
			const LightDesc* lightDesc = (const LightDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++lightDesc)
			{
				Matrix4x4 transformMatrix = world.sceneGraph->getWorldPose(unitLookupMap[unitIndexList[i]]);
				world.renderWorld->lightCreate(unitLookupMap[unitIndexList[i]], *lightDesc, transformMatrix);
			}
		}
#if AMSTEL_ENGINE_SCRIPT_LUA
		else if (componentData->type == COMPONENT_TYPE_SCRIPT)
		{
			const ScriptDesc* scriptDescList = (const ScriptDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++scriptDescList)
			{
				ScriptWorldFn::create(*scriptWorld, unitLookupMap[unitIndexList[i]], *scriptDescList);
			}
		}
#endif // AMSTEL_ENGINE_SCRIPT_LUA
		else if (componentData->type == COMPONENT_TYPE_ANIMATION_STATE_MACHINE)
		{
			const AnimationStateMachineDesc* animationStateMachineDescList = (const AnimationStateMachineDesc*)componentDataBufferBegin;
			for (uint32_t i = 0, n = componentData->instanceListCount; i < n; ++i, ++animationStateMachineDescList)
			{
				world.animationStateMachine->create(unitLookupMap[unitIndexList[i]], *animationStateMachineDescList);
			}
		}
		else
		{
			RIO_FATAL("Unknown component type");
		}
	}

	for (uint32_t i = 0; i < unitResource.unitListCount; ++i)
	{
		ArrayFn::pushBack(world.unitIdList, unitLookupMap[i]);
	}

	// Post events
	for (uint32_t i = 0; i < unitResource.unitListCount; ++i)
	{
		world.postUnitSpawnedEvent(unitLookupMap[i]);
	}
}

} // namespace Rio
