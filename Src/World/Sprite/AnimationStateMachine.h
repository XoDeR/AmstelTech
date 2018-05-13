#pragma once

#include "Core/Containers/EventStream.h"
#include "Core/Containers/Types.h"

#include "Resource/Sprite/StateMachineResource.h"
#include "Resource/Types.h"

#include "World/Types.h"

namespace Rio
{

struct SpriteFrameChangeEvent
{
	UnitId unitId;
	uint32_t frameId = UINT32_MAX;
};

struct AnimationStateMachine
{
	struct Animation
	{
		UnitId unitId;
		float totalTime = 0.0f;
		float time = 0.0f;
		uint32_t frameListCount = 0;
		const uint32_t* frameList = nullptr;
		const SpriteAnimationResource* spriteAnimationResource = nullptr;
		const State* state = nullptr;
		const State* stateNext = nullptr;
		const StateMachineResource* stateMachineResource = nullptr;
		float* variableList = nullptr;
	};

	uint32_t marker = ANIMATION_STATE_MACHINE_MARKER;
	ResourceManager* resourceManager = nullptr;
	UnitManager* unitManager = nullptr;

	HashMap<UnitId, uint32_t> animationIndexMap;
	Array<Animation> animationList;
	EventStream eventStream;

	AnimationStateMachine(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager);
	~AnimationStateMachine();

	uint32_t create(UnitId unitId, const AnimationStateMachineDesc& animationStateMachineDesc);
	void destroy(UnitId unitId);
	uint32_t getAnimationInstanceByUnitId(UnitId unitId);
	bool has(UnitId unitId);
	uint32_t getVariableIdByName(UnitId unitId, StringId32 name);
	float getVariableById(UnitId unitId, uint32_t variableId);
	void setVariable(UnitId unitId, uint32_t variableId, float floatValue);
	void trigger(UnitId unitId, StringId32 eventName);
	void update(float dt);
	void unitDestroyedCallback(UnitId unitId);
};

} // namespace Rio
