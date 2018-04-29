#include "Resource/Sprite/StateMachineResource.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/Containers/Map.h"

#include "Core/Error/Error.h"

#include "Core/FileSystem/File.h"

#include "Core/Guid.h"

#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"

#include "Core/Math/Math.h"

#include "Core/Memory/TempAllocator.h"

#include "Core/Strings/DynamicString.h"

#include "Resource/CompileOptions.h"
#include "Resource/Sprite/ExpressionLanguage.h"
#include "Resource/Types.h"

namespace Rio
{

template <>
struct Hash<Guid>
{
	uint32_t operator()(const Guid& guid) const
	{
		return guid.data1;
	}
};

namespace StateMachineInternalFn
{
	struct TransitionModeInfo
	{
		const char* name = nullptr;
		TransitionMode::Enum mode = TransitionMode::COUNT;
	};

	static TransitionModeInfo transitionModeInfoMapStatic[] =
	{
		{ "immediate", TransitionMode::IMMEDIATE },
		{ "waitUntilEnd", TransitionMode::WAIT_UNTIL_END },
	};
	RIO_STATIC_ASSERT(countof(transitionModeInfoMapStatic) == TransitionMode::COUNT);

	static TransitionMode::Enum getTransitionModeByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(transitionModeInfoMapStatic); ++i)
		{
			if (strcmp(name, transitionModeInfoMapStatic[i].name) == 0)
			{
				return transitionModeInfoMapStatic[i].mode;
			}
		}

		return TransitionMode::COUNT;
	}

	struct OffsetAccumulator
	{
		uint32_t offset = 0;

		OffsetAccumulator()
		{
			this->offset = sizeof(StateMachineResource);
		}

		// Returns the offset of
		uint32_t getOffset(uint32_t animationListCount, uint32_t transitionListCount)
		{
			const uint32_t offset = this->offset;
			this->offset += sizeof(State);
			this->offset += sizeof(Transition) * transitionListCount;
			this->offset += sizeof(AnimationArray);
			this->offset += sizeof(Animation) * animationListCount;
			return offset;
		}
	};

	struct AnimationInfo
	{
		ALLOCATOR_AWARE;

		StringId64 name;
		DynamicString weight;
		uint32_t byteCodeEntry = 0;

		AnimationInfo(Allocator& a)
			: weight(a)
		{
		}
	};

	struct TransitionInfo
	{
		Transition transition;
		Guid stateGuid;
	};

	struct StateInfo
	{
		ALLOCATOR_AWARE;

		Vector<AnimationInfo> animationInfoList;
		Vector<TransitionInfo> transitionInfoList;
		DynamicString speedString;
		uint32_t speedByteCode = UINT32_MAX;
		uint32_t loop = 0;

		StateInfo()
			: animationInfoList(getDefaultAllocator())
			, transitionInfoList(getDefaultAllocator())
			, speedString(getDefaultAllocator())
		{
		}

		StateInfo(Allocator& a)
			: animationInfoList(a)
			, transitionInfoList(a)
			, speedString(a)
		{
		}
	};

	struct VariableInfo
	{
		ALLOCATOR_AWARE;

		DynamicString variableNameString;
		StringId32 name;
		float value = 0.0f;

		VariableInfo()
			: variableNameString(getDefaultAllocator())
		{
		}

		VariableInfo(Allocator& a)
			: variableNameString(a)
		{
		}
	};

	struct StateMachineCompiler
	{
		CompileOptions compileOptions;
		Guid initialStateGuid;
		Map<Guid, StateInfo> stateInfoMap;
		OffsetAccumulator offsetAccumulator;
		HashMap<Guid, uint32_t> stateInfoGuidOffsetMap;
		Vector<VariableInfo> variableInfoList;
		Array<uint32_t> byteCodeBuffer;

		StateMachineCompiler(CompileOptions compileOptions)
			: compileOptions(compileOptions)
			, stateInfoMap(getDefaultAllocator())
			, stateInfoGuidOffsetMap(getDefaultAllocator())
			, variableInfoList(getDefaultAllocator())
			, byteCodeBuffer(getDefaultAllocator())
		{
		}

		void parse(Buffer& buffer)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject animationStateMachineJsonObject(tempAllocator4096);
			JsonObject stateListJsonObject(tempAllocator4096);
			JsonObject variableListJsonObject(tempAllocator4096);

			RJsonFn::parse(buffer, animationStateMachineJsonObject);
			RJsonFn::parseObject(animationStateMachineJsonObject["stateList"], stateListJsonObject);
			RJsonFn::parseObject(animationStateMachineJsonObject["variableList"], variableListJsonObject);

			// Parse states
			auto currentStatePair = JsonObjectFn::begin(stateListJsonObject);
			auto endStatePair = JsonObjectFn::end(stateListJsonObject);
			for (; currentStatePair != endStatePair; ++currentStatePair)
			{
				const FixedString key = currentStatePair->pair.first;

				TempAllocator4096 tempAllocator4096;
				JsonObject stateJsonObject(tempAllocator4096);
				JsonObject animationListJsonObject(tempAllocator4096);
				JsonObject transitionListJsonObject(tempAllocator4096);
				RJsonFn::parseObject(stateListJsonObject[key], stateJsonObject);
				RJsonFn::parseObject(stateJsonObject["animationList"], animationListJsonObject);
				RJsonFn::parseObject(stateJsonObject["transitionList"], transitionListJsonObject);

				StateInfo stateInfo;

				RJsonFn::parseString(stateJsonObject["speed"], stateInfo.speedString);
				stateInfo.loop = RJsonFn::parseBool(stateJsonObject["loop"]);

				// Parse transitions
				{
					auto currentTransitionPair = JsonObjectFn::begin(transitionListJsonObject);
					auto endTransitionPair = JsonObjectFn::end(transitionListJsonObject);
					for (; currentTransitionPair != endTransitionPair; ++currentTransitionPair)
					{
						const FixedString key = currentTransitionPair->pair.first;

						JsonObject transitionJsonObject(tempAllocator4096);
						RJsonFn::parseObject(transitionListJsonObject[key], transitionJsonObject);

						DynamicString modeString(tempAllocator4096);
						RJsonFn::parseString(transitionJsonObject["mode"], modeString);
						const uint32_t mode = getTransitionModeByName(modeString.getCStr());
						DATA_COMPILER_ASSERT(mode != TransitionMode::COUNT
							, this->compileOptions
							, "Unknown transition mode: '%s'"
							, modeString.getCStr()
							);

						TransitionInfo transitionInfo;
						transitionInfo.transition.event = RJsonFn::parseStringId(transitionJsonObject["event"]);
						transitionInfo.transition.transitionStateOffset = UINT32_MAX;
						transitionInfo.transition.mode = mode;
						transitionInfo.stateGuid = RJsonFn::parseGuid(transitionJsonObject["to"]);

						VectorFn::pushBack(stateInfo.transitionInfoList, transitionInfo);
					}
				}

				// Parse animations
				{
					auto currentAnimationPair = JsonObjectFn::begin(animationListJsonObject);
					auto endAnimationPair = JsonObjectFn::end(animationListJsonObject);
					for (; currentAnimationPair != endAnimationPair; ++currentAnimationPair)
					{
						const FixedString key = currentAnimationPair->pair.first;

						JsonObject animationJsonObject(tempAllocator4096);
						RJsonFn::parseObject(animationListJsonObject[key], animationJsonObject);

						DynamicString spriteAnimationResourceName(tempAllocator4096);
						RJsonFn::parseString(animationJsonObject["name"], spriteAnimationResourceName);
						DATA_COMPILER_ASSERT_RESOURCE_EXISTS("spriteAnimation"
							, spriteAnimationResourceName.getCStr()
							, this->compileOptions
							);

						AnimationInfo animationInfo(tempAllocator4096);
						animationInfo.name = RJsonFn::parseResourceId(animationJsonObject["name"]);
						RJsonFn::parseString(animationJsonObject["weight"], animationInfo.weight);

						VectorFn::pushBack(stateInfo.animationInfoList, animationInfo);
					}
					DATA_COMPILER_ASSERT(VectorFn::getCount(stateInfo.animationInfoList) > 0
						, this->compileOptions
						, "State must contain one animation at least"
						);
				}

				Guid guid = GuidFn::parseFromString(key.getCStr());
				DATA_COMPILER_ASSERT(!MapFn::has(stateInfoMap, guid)
					, this->compileOptions
					, "State GUID duplicated"
					);
				MapFn::set(stateInfoMap, guid, stateInfo);
			}
			DATA_COMPILER_ASSERT(MapFn::getCount(stateInfoMap) > 0
				, this->compileOptions
				, "State machine must contain one state at least"
				);

			initialStateGuid = RJsonFn::parseGuid(animationStateMachineJsonObject["initialState"]);
			DATA_COMPILER_ASSERT(MapFn::has(stateInfoMap, initialStateGuid)
				, this->compileOptions
				, "Initial state references non-existing state"
				);

			// Parse variables
			{
				auto currentVariablePair = JsonObjectFn::begin(variableListJsonObject);
				auto endVariablePair = JsonObjectFn::end(variableListJsonObject);
				for (; currentVariablePair != endVariablePair; ++currentVariablePair)
				{
					JsonObject variableJsonObject(tempAllocator4096);
					RJsonFn::parseObject(variableListJsonObject[currentVariablePair->pair.first], variableJsonObject);

					VariableInfo variableInfo;
					variableInfo.name = RJsonFn::parseStringId(variableJsonObject["name"]);
					variableInfo.value = RJsonFn::parseFloat(variableJsonObject["value"]);
					RJsonFn::parseString(variableJsonObject["name"], variableInfo.variableNameString);

					VectorFn::pushBack(variableInfoList, variableInfo);
				}
			}

			// Compute state offsets
			{
				// Limit byte code to 4K
				ArrayFn::resize(byteCodeBuffer, 1024);
				uint32_t written = 0;

				const uint32_t variableListCount = VectorFn::getCount(variableInfoList);
				const char** variableNameList = (const char**)getDefaultAllocator().allocate(variableListCount *sizeof(char*));

				for (uint32_t i = 0; i < variableListCount; ++i)
				{
					variableNameList[i] = variableInfoList[i].variableNameString.getCStr();
				}

				const uint32_t constantListCount = 1;
				
				const char* constantNameList[] =
				{
					"PI"
				};

				const float constantValueList[] =
				{
					PI
				};

				auto currentStateInfoPair = MapFn::begin(stateInfoMap);
				auto endStateInfoPair = MapFn::end(stateInfoMap);
				for (; currentStateInfoPair != endStateInfoPair; ++currentStateInfoPair)
				{
					const Guid& guid = currentStateInfoPair->pair.first;
					const StateInfo& stateInfo = currentStateInfoPair->pair.second;

					const uint32_t offset = offsetAccumulator.getOffset(VectorFn::getCount(stateInfo.animationInfoList), VectorFn::getCount(stateInfo.transitionInfoList));
					HashMapFn::set(stateInfoGuidOffsetMap, guid, offset);

					for (uint32_t i = 0; i < VectorFn::getCount(stateInfo.animationInfoList); ++i)
					{
						const uint32_t weightCompiledCount = ExpressionLanguageFn::compile(stateInfo.animationInfoList[i].weight.getCStr()
							 , variableListCount
							 , variableNameList
							 , constantListCount
							 , constantNameList
							 , constantValueList
							 , ArrayFn::begin(byteCodeBuffer) + written
							 , ArrayFn::getCount(byteCodeBuffer)
							 );

						const_cast<AnimationInfo&>(stateInfo.animationInfoList[i]).byteCodeEntry = weightCompiledCount > 0 ? written : UINT32_MAX;
						written += weightCompiledCount;
					}

					const uint32_t speedCompiledCount = ExpressionLanguageFn::compile(stateInfo.speedString.getCStr()
						 , variableListCount
						 , variableNameList
						 , constantListCount
						 , constantNameList
						 , constantValueList
						 , ArrayFn::begin(byteCodeBuffer) + written
						 , ArrayFn::getCount(byteCodeBuffer)
						 );

					const_cast<StateInfo&>(stateInfo).speedByteCode = speedCompiledCount > 0 ? written : UINT32_MAX;
					written += speedCompiledCount;
				}

				getDefaultAllocator().deallocate(variableNameList);
			}
		}

		void write(CompileOptions& compileOptions)
		{
			StateMachineResource stateMachineResource;
			stateMachineResource.version = RESOURCE_VERSION_STATE_MACHINE;
			stateMachineResource.initialStateOffset = HashMapFn::get(stateInfoGuidOffsetMap, initialStateGuid, UINT32_MAX);
			stateMachineResource.variableListCount = VectorFn::getCount(variableInfoList);
			stateMachineResource.variableListOffset = offsetAccumulator.offset; // Offset of last state + 1
			stateMachineResource.byteCodeSize = ArrayFn::getCount(byteCodeBuffer) * 4;
			stateMachineResource.byteCodeOffset = stateMachineResource.variableListOffset + stateMachineResource.variableListCount * 4 * 2;

			compileOptions.write(stateMachineResource.version);
			compileOptions.write(stateMachineResource.initialStateOffset);
			compileOptions.write(stateMachineResource.variableListCount);
			compileOptions.write(stateMachineResource.variableListOffset);
			compileOptions.write(stateMachineResource.byteCodeSize);
			compileOptions.write(stateMachineResource.byteCodeOffset);

			// Write states
			auto currentStateInfoPair = MapFn::begin(stateInfoMap);
			auto endStateInfoPair = MapFn::end(stateInfoMap);
			for (; currentStateInfoPair != endStateInfoPair; ++currentStateInfoPair)
			{
				const StateInfo& stateInfo = currentStateInfoPair->pair.second;
				const uint32_t animationInfoListCount  = VectorFn::getCount(stateInfo.animationInfoList);
				const uint32_t transitionInfoListCount = VectorFn::getCount(stateInfo.transitionInfoList);

				// Write speed
				compileOptions.write(stateInfo.speedByteCode);

				// Write loop
				compileOptions.write(stateInfo.loop);

				// Write transitions
				TransitionArray transitionArray;
				transitionArray.transitionListCount = transitionInfoListCount;
				compileOptions.write(transitionArray.transitionListCount);
				for (uint32_t i = 0; i < transitionInfoListCount; ++i)
				{
					Transition transitionCurrent = stateInfo.transitionInfoList[i].transition;
					transitionCurrent.transitionStateOffset = HashMapFn::get(stateInfoGuidOffsetMap, stateInfo.transitionInfoList[i].stateGuid, UINT32_MAX);

					compileOptions.write(transitionCurrent.event);
					compileOptions.write(transitionCurrent.transitionStateOffset);
					compileOptions.write(transitionCurrent.mode);
				}

				// Write animations
				AnimationArray animationArray;
				animationArray.animationListCount = animationInfoListCount;
				compileOptions.write(animationArray.animationListCount);
				for (uint32_t i = 0; i < animationInfoListCount; ++i)
				{
					Animation animation;
					animation.name = stateInfo.animationInfoList[i].name;
					animation.byteCodeEntry = stateInfo.animationInfoList[i].byteCodeEntry;
					animation.padding00 = 0;

					compileOptions.write(animation.name);
					compileOptions.write(animation.byteCodeEntry);
					compileOptions.write(animation.padding00);
				}
			}

			// Write variables
			for (uint32_t i = 0; i < VectorFn::getCount(variableInfoList); ++i)
			{
				compileOptions.write(variableInfoList[i].name);
			}
			for (uint32_t i = 0; i < VectorFn::getCount(variableInfoList); ++i)
			{
				compileOptions.write(variableInfoList[i].value);
			}

			// Write bytecode
			for (uint32_t i = 0; i < ArrayFn::getCount(byteCodeBuffer); ++i)
			{
				compileOptions.write(byteCodeBuffer[i]);
			}
		}
	};

	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		StateMachineCompiler stateMachineCompiler(compileOptions);
		stateMachineCompiler.parse(buffer);
		stateMachineCompiler.write(compileOptions);
	}

} // namespace StateMachineInternalFn

namespace StateMachineFn
{
	const State* getInitialState(const StateMachineResource* stateMachineResource)
	{
		return (State*)((char*)stateMachineResource + stateMachineResource->initialStateOffset);
	}

	const State* getStateByTransition(const StateMachineResource* stateMachineResource, const Transition* transition)
	{
		return (State*)((char*)stateMachineResource + transition->transitionStateOffset);
	}

	const State* trigger(const StateMachineResource* stateMachineResource, const State* state, StringId32 event, const Transition** transitionResult)
	{
		const TransitionArray* transitionArray = getStateTransitionList(state);

		for (uint32_t i = 0; i < transitionArray->transitionListCount; ++i)
		{
			const Transition* transitionCurrent = getTransitionByIndex(transitionArray, i);

			if (transitionCurrent->event == event)
			{
				*transitionResult = transitionCurrent;
				return getStateByTransition(stateMachineResource, transitionCurrent);
			}
		}

		*transitionResult = nullptr;
		return state;
	}

	const TransitionArray* getStateTransitionList(const State* state)
	{
		return &state->transitionArray;
	}

	const Transition* getTransitionByIndex(const TransitionArray* transitionArray, uint32_t index)
	{
		RIO_ASSERT(index < transitionArray->transitionListCount, "Index out of bounds");
		const Transition* firstTransition = (Transition*)(&transitionArray[1]);
		return &firstTransition[index];
	}

	const AnimationArray* getStateAnimationList(const State* s)
	{
		const TransitionArray* transitionArray = getStateTransitionList(s);
		const Transition* firstTransition = (Transition*)(&transitionArray[1]);
		return (AnimationArray*)(firstTransition + transitionArray->transitionListCount);
	}

	const Animation* getAnimationByIndex(const AnimationArray* animationArray, uint32_t index)
	{
		RIO_ASSERT(index < animationArray->animationListCount, "Index out of bounds");
		Animation* firstAnimation = (Animation*)(&animationArray[1]);
		return &firstAnimation[index];
	}

	static inline const StringId32* getVariableNameList(const StateMachineResource* stateMachineResource)
	{
		return (StringId32*)((char*)stateMachineResource + stateMachineResource->variableListOffset);
	}

	const float* getVariableList(const StateMachineResource* stateMachineResource)
	{
		const StringId32* variableNameList = getVariableNameList(stateMachineResource);
		return (float*)(variableNameList + stateMachineResource->variableListCount);
	}

	uint32_t getVariableIndexByName(const StateMachineResource* stateMachineResource, StringId32 name)
	{
		const StringId32* variableNameList = getVariableNameList(stateMachineResource);
		for (uint32_t i = 0; i < stateMachineResource->variableListCount; ++i)
		{
			if (variableNameList[i] == name)
			{
				return i;
			}
		}

		return UINT32_MAX;
	}

	const uint32_t* getByteCode(const StateMachineResource* stateMachineResource)
	{
		return (uint32_t*)((char*)stateMachineResource + stateMachineResource->byteCodeOffset);
	}

} // namespace StateMachineFn

} // namespace Rio
