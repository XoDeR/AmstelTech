#include "World/Sprite/AnimationStateMachine.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/Containers/Types.h"

#include "Resource/Sprite/ExpressionLanguage.h"
#include "Resource/ResourceManager.h"
#include "Resource/Sprite/SpriteResource.h"
#include "Resource/Sprite/StateMachineResource.h"

#include "World/Types.h"
#include "World/UnitManager.h"

namespace Rio
{

static void unitDestroyedCallbackBridge(UnitId unitId, void* userPtr)
{
	((AnimationStateMachine*)userPtr)->unitDestroyedCallback(unitId);
}

AnimationStateMachine::AnimationStateMachine(Allocator& a, ResourceManager& resourceManager, UnitManager& unitManager)
	: resourceManager(&resourceManager)
	, unitManager(&unitManager)
	, animationIndexMap(a)
	, animationList(a)
	, eventStream(a)
{
	unitManager.registerDestroyFunction(unitDestroyedCallbackBridge, this);
}

AnimationStateMachine::~AnimationStateMachine()
{
	unitManager->unregisterDestroyFunction(this);
	marker = 0;
}

uint32_t AnimationStateMachine::create(UnitId unitId, const AnimationStateMachineDesc& animationStateMachineDesc)
{
	RIO_ASSERT(!HashMapFn::has(this->animationIndexMap, unitId), "Unit already has this component");

	const StateMachineResource* stateMachineResource = (StateMachineResource*)resourceManager->getResourceData(RESOURCE_TYPE_STATE_MACHINE, animationStateMachineDesc.stateMachineResource);

	Animation animation;
	animation.unitId = unitId;
	animation.time = 0.0f;
	animation.totalTime = 0.0f;
	animation.frameListCount = 0;
	animation.frameList = nullptr;
	animation.spriteAnimationResource = nullptr;
	animation.state = StateMachineFn::getInitialState(stateMachineResource);
	animation.stateNext = nullptr;
	animation.stateMachineResource = stateMachineResource;
	animation.variableList = (float*)getDefaultAllocator().allocate(sizeof(*animation.variableList) * stateMachineResource->variableListCount);

	memcpy(animation.variableList, StateMachineFn::getVariableList(stateMachineResource), sizeof(*animation.variableList)*stateMachineResource->variableListCount);

	uint32_t lastAnimation = ArrayFn::getCount(this->animationList);
	ArrayFn::pushBack(this->animationList, animation);
	HashMapFn::set(this->animationIndexMap, unitId, lastAnimation);

	return 0;
}

void AnimationStateMachine::destroy(UnitId unitId)
{
	const uint32_t animationIndex = HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);
	const uint32_t lastAnimationIndex = ArrayFn::getCount(this->animationList) - 1;
	const UnitId lastUnitId = this->animationList[lastAnimationIndex].unitId;

	getDefaultAllocator().deallocate(this->animationList[animationIndex].variableList);
	this->animationList[animationIndex] = this->animationList[lastAnimationIndex];

	ArrayFn::popBack(this->animationList);
	HashMapFn::set(this->animationIndexMap, lastUnitId, animationIndex);
	HashMapFn::remove(this->animationIndexMap, unitId);
}

uint32_t AnimationStateMachine::getAnimationInstanceByUnitId(UnitId unitId)
{
	return HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);
}

bool AnimationStateMachine::has(UnitId unitId)
{
	return HashMapFn::has(this->animationIndexMap, unitId);
}

uint32_t AnimationStateMachine::getVariableIdByName(UnitId unitId, StringId32 name)
{
	const uint32_t animationIndex = HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);
	const uint32_t variableId = StateMachineFn::getVariableIndexByName(this->animationList[animationIndex].stateMachineResource, name);
	return variableId;
}

float AnimationStateMachine::getVariableById(UnitId unitId, uint32_t variableId)
{
	const uint32_t i = HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);
	RIO_ENSURE(variableId != UINT32_MAX);
	return this->animationList[i].variableList[variableId];
}

void AnimationStateMachine::setVariable(UnitId unitId, uint32_t variableId, float floatValue)
{
	const uint32_t i = HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);
	RIO_ENSURE(variableId != UINT32_MAX);
	this->animationList[i].variableList[variableId] = floatValue;
}

void AnimationStateMachine::trigger(UnitId unitId, StringId32 eventName)
{
	const uint32_t i = HashMapFn::get(this->animationIndexMap, unitId, UINT32_MAX);

	const Transition* transition = nullptr;
	const State* state = StateMachineFn::trigger(this->animationList[i].stateMachineResource
		, this->animationList[i].state
		, eventName
		, &transition
		);

	if (transition == nullptr)
	{
		return;
	}

	if (transition->mode == TransitionMode::IMMEDIATE)
	{
		this->animationList[i].state = state;
	}
	else if (transition->mode == TransitionMode::WAIT_UNTIL_END)
	{
		this->animationList[i].stateNext = state;
	}
	else
	{
		RIO_FATAL("Unknown transition mode");
	}
}

void AnimationStateMachine::update(float dt)
{
	const uint32_t stackSize = 32;
	float stackData[stackSize];

	ExpressionLanguageFn::Stack stack(stackData, stackSize);

	for (uint32_t i = 0; i < ArrayFn::getCount(this->animationList); ++i)
	{
		Animation& animationCurrent = this->animationList[i];

		const float* variableList = animationCurrent.variableList;
		const uint32_t* byteCode = StateMachineFn::getByteCode(animationCurrent.stateMachineResource);

		// Evaluate animation weights
		float maxValueFloat = 0.0f;
		uint32_t maxIndex = UINT32_MAX;
		StringId64 name;

		const AnimationArray* animationArray = StateMachineFn::getStateAnimationList(animationCurrent.state);
		for (uint32_t i = 0; i < animationArray->animationListCount; ++i)
		{
			const Rio::Animation* animation = StateMachineFn::getAnimationByIndex(animationArray, i);

			stack.size = 0;
			ExpressionLanguageFn::run(&byteCode[animation->byteCodeEntry], variableList, stack);
			const float currentValueFloat = stack.size > 0 ? stackData[stack.size - 1] : 0.0f;
			if (currentValueFloat > maxValueFloat || maxIndex == UINT32_MAX)
			{
				maxValueFloat = currentValueFloat;
				maxIndex = i;
				name = animation->name;
			}
		}

		// Evaluate animation speed
		stack.size = 0;
		ExpressionLanguageFn::run(&byteCode[animationCurrent.state->speedByteCode], variableList, stack);
		const float speed = stack.size > 0 ? stackData[stack.size - 1] : 1.0f;

		// Play animation
		const SpriteAnimationResource* spriteAnimationResource = (SpriteAnimationResource*)resourceManager->getResourceData(RESOURCE_TYPE_SPRITE_ANIMATION, name);
		if (animationCurrent.spriteAnimationResource != spriteAnimationResource)
		{
			animationCurrent.time = 0.0f;
			animationCurrent.totalTime = spriteAnimationResource->totalTime;
			animationCurrent.frameListCount = spriteAnimationResource->frameListCount;
			animationCurrent.frameList = SpriteAnimationResourceFn::getSpriteAnimationFrameList(spriteAnimationResource);
			animationCurrent.spriteAnimationResource = spriteAnimationResource;
		}

		if (animationCurrent.spriteAnimationResource == nullptr)
		{
			continue;
		}

		const float animationFrameTime  = float(animationCurrent.frameListCount) * (animationCurrent.time / animationCurrent.totalTime);
		const uint32_t animationFrameIndex = uint32_t(animationFrameTime) % animationCurrent.frameListCount;

		animationCurrent.time += dt * speed;

		// If animation finished playing
		if (animationCurrent.time > animationCurrent.totalTime)
		{
			if (animationCurrent.stateNext)
			{
				animationCurrent.state = animationCurrent.stateNext;
				animationCurrent.stateNext = nullptr;
				animationCurrent.time = 0.0f;
			}
			else
			{
				if (!!animationCurrent.state->loop)
				{
					animationCurrent.time = animationCurrent.time - animationCurrent.totalTime;
				}
				else
				{
					const Transition* transitionDummy = nullptr;
					const State* animationStateLocal = StateMachineFn::trigger(animationCurrent.stateMachineResource
						, animationCurrent.state
						, StringId32("animationEnd")
						, &transitionDummy
						);
					animationCurrent.time = animationCurrent.state != animationStateLocal ? 0.0f : animationCurrent.totalTime;
					animationCurrent.state = animationStateLocal;
				}
			}
		}

		// Emit events
		SpriteFrameChangeEvent spriteFrameChangeEvent;
		spriteFrameChangeEvent.unitId = animationCurrent.unitId;
		spriteFrameChangeEvent.frameId = animationCurrent.frameList[animationFrameIndex];
		EventStreamFn::write(this->eventStream, 0, spriteFrameChangeEvent);
	}
}

void AnimationStateMachine::unitDestroyedCallback(UnitId unitId)
{
	if (has(unitId))
	{
		destroy(unitId);
	}
}

} // namespace Rio
