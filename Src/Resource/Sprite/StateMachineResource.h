#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

namespace Rio
{

struct StateMachineResource
{
	uint32_t version = UINT32_MAX;;
	uint32_t initialStateOffset = 0;
	uint32_t variableListCount = 0;
	uint32_t variableListOffset = 0;
	uint32_t byteCodeSize = 0;
	uint32_t byteCodeOffset = 0;
};

struct AnimationArray
{
	uint32_t animationListCount = 0;
};

struct Animation
{
	StringId64 name;
	uint32_t byteCodeEntry = 0;
	uint32_t padding00 = 0;
};

struct TransitionArray
{
	uint32_t transitionListCount = 0;
};

struct Transition
{
	StringId32 event;
	uint32_t transitionStateOffset = UINT32_MAX;
	uint32_t mode = UINT32_MAX; // TransitionMode::Enum
};

struct State
{
	uint32_t speedByteCode = 0;
	uint32_t loop = 0;
	TransitionArray transitionArray;
};

struct TransitionMode
{
	enum Enum
	{
		IMMEDIATE,
		WAIT_UNTIL_END,

		COUNT
	};
};

namespace StateMachineInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace StateMachineInternalFn

namespace StateMachineFn
{
	// Returns the initial state
	const State* getInitialState(const StateMachineResource* stateMachineResource);

	// Returns the state pointed to by transition <transition>
	const State* getStateByTransition(const StateMachineResource* stateMachineResource, const Transition* transition);

	// Triggers the transition <event> in the state <state> and returns the resulting state
	// If no transition with <event> is found it returns the state <state>
	const State* trigger(const StateMachineResource* stateMachineResource, const State* state, StringId32 event, const Transition** transitionResult);

	// Returns the transitions for the state <state>
	const TransitionArray* getStateTransitionList(const State* state);

	// Returns the transition with given <index>
	const Transition* getTransitionByIndex(const TransitionArray* transitionArray, uint32_t index);

	// Returns the animations for the state <state>
	const AnimationArray* getStateAnimationList(const State* state);

	// Returns the animation with the given <index>
	const Animation* getAnimationByIndex(const AnimationArray* animationArray, uint32_t index);

	// Returns the variables of the state machine
	const float* getVariableList(const StateMachineResource* stateMachineResource);

	// Returns the index of the variable <name> or UINT32_MAX if not found
	uint32_t getVariableIndexByName(const StateMachineResource* stateMachineResource, StringId32 name);

	// Returns the byte code of the state machine
	const uint32_t* getByteCode(const StateMachineResource* stateMachineResource);

} // namespace StateMachineFn

} // namespace Rio
