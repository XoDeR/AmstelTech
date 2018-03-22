#include "Core/Profiler.h"

#include "Core/Containers/Array.h"
#include "Core/Math/Vector3.h"
#include "Core/Memory/Memory.h"
#include "Core/Os.h"
#include "Core/Thread/Mutex.h"

namespace Rio
{

namespace ProfilerGlobalFn
{
	char memoryToAllocate[sizeof(Buffer)];
	Buffer* buffer = nullptr;

	void init()
	{
		buffer = new (memoryToAllocate)Buffer(getDefaultAllocator());
	}

	void shutdown()
	{
		buffer->~Buffer();
		buffer = nullptr;
	}

	const char* getBuffer()
	{
		return ArrayFn::begin(*buffer);
	}

} // namespace ProfilerGlobalFn

namespace ProfilerFn
{
	enum 
	{ 
		THREAD_BUFFER_SIZE = 4 * 1024 
	};

	static char threadBufferMemory[THREAD_BUFFER_SIZE];
	static uint32_t threadBufferSize = 0;
	static Mutex threadBufferMutex;

	static void flushLocalBuffer()
	{
		ScopedMutex scopedMutex(threadBufferMutex);
		ArrayFn::push(*ProfilerGlobalFn::buffer, threadBufferMemory, threadBufferSize);
		threadBufferSize = 0;
	}

	template <typename T>
	static void push(ProfilerEventType::Enum type, const T& profilerEvent)
	{
		if (threadBufferSize + 2 * sizeof(uint32_t) + sizeof(profilerEvent) >= THREAD_BUFFER_SIZE)
		{
			flushLocalBuffer();
		}

		char* p = threadBufferMemory + threadBufferSize;
		*(uint32_t*)p = type;
		p += sizeof(uint32_t);
		*(uint32_t*)p = sizeof(profilerEvent);
		p += sizeof(uint32_t);
		*(T*)p = profilerEvent;

		threadBufferSize += 2*sizeof(uint32_t) + sizeof(profilerEvent);
	}

	void enterProfileScope(const char* name)
	{
		EnterProfileScope profilerEvent;
		profilerEvent.name = name;
		profilerEvent.time = OsFn::getClockTime();

		push(ProfilerEventType::ENTER_PROFILE_SCOPE, profilerEvent);
	}

	void leaveProfileScope()
	{
		LeaveProfileScope profilerEvent;
		profilerEvent.time = OsFn::getClockTime();

		push(ProfilerEventType::LEAVE_PROFILE_SCOPE, profilerEvent);
	}

	void recordFloat(const char* name, float value)
	{
		RecordFloat profilerEvent;
		profilerEvent.name = name;
		profilerEvent.value = value;

		push(ProfilerEventType::RECORD_FLOAT, profilerEvent);
	}

	void recordVector3(const char* name, const Vector3& value)
	{
		RecordVector3 profilerEvent;
		profilerEvent.name = name;
		profilerEvent.value = value;

		push(ProfilerEventType::RECORD_VECTOR3, profilerEvent);
	}

	void allocateMemory(const char* name, uint32_t size)
	{
		AllocateMemory profilerEvent;
		profilerEvent.name = name;
		profilerEvent.size = size;

		push(ProfilerEventType::ALLOCATE_MEMORY, profilerEvent);
	}

	void deallocateMemory(const char* name, uint32_t size)
	{
		DeallocateMemory profilerEvent;
		profilerEvent.name = name;
		profilerEvent.size = size;

		push(ProfilerEventType::DEALLOCATE_MEMORY, profilerEvent);
	}

} // namespace ProfilerFn

namespace ProfilerGlobalFn
{
	void flush()
	{
		ProfilerFn::flushLocalBuffer();
		uint32_t end = ProfilerEventType::COUNT;
		ArrayFn::push(*buffer, (const char*)&end, (uint32_t)sizeof(end));
	}

	void clear()
	{
		ArrayFn::clear(*buffer);
	}

} // namespace ProfilerGlobalFn

} // namespace Rio
