#pragma once

#include "Core/Math/Types.h"
#include "Core/Types.h"

namespace Rio
{

struct ProfilerEventType
{
	enum Enum
	{
		ENTER_PROFILE_SCOPE,
		LEAVE_PROFILE_SCOPE,
		RECORD_FLOAT,
		RECORD_VECTOR3,
		ALLOCATE_MEMORY,
		DEALLOCATE_MEMORY,

		COUNT
	};
};

struct RecordFloat
{
	const char* name = nullptr;
	float value = 0.0f;
};

struct RecordVector3
{
	const char* name = nullptr;
	Vector3 value;
};

struct EnterProfileScope
{
	const char* name = nullptr;
	int64_t time = 0;
};

struct LeaveProfileScope
{
	int64_t time = 0;
};

struct AllocateMemory
{
	const char* name = nullptr;
	uint32_t size = 0;
};

struct DeallocateMemory
{
	const char* name = nullptr;
	uint32_t size = 0;
};

// The profiler does not copy pointer data
// Need to store it somewhere and make sure it is valid throughout the program execution
namespace ProfilerFn
{
	// Starts a new profile scope with the given <name>
	void enterProfileScope(const char* name);

	// Ends the last profile scope
	void leaveProfileScope();

	// Records the float <value> with the given <name>
	void recordFloat(const char* name, float value);

	// Records the Vector3 <value> with the given <name>
	void recordVector3(const char* name, const Vector3& value);

	// Records a memory allocation of <size> with the given <name>
	void allocateMemory(const char* name, uint32_t size);

	// Records a memory deallocation of <size> with the given <name>
	void deallocateMemory(const char* name, uint32_t size);

} // namespace ProfilerFn

namespace ProfilerGlobalFn
{
	void init();
	void shutdown();

	const char* getBuffer();
	void flush();
	void clear();

} // namespace ProfilerGlobalFn

} // namespace Rio

#if RIO_DEBUG
	#define ENTER_PROFILE_SCOPE(name) ProfilerFn::enterProfileScope(name)
	#define LEAVE_PROFILE_SCOPE() ProfilerFn::leaveProfileScope()
	#define RECORD_FLOAT(name, value) ProfilerFn::recordFloat(name, value)
	#define RECORD_VECTOR3(name, value) ProfilerFn::recordVector3(name, value)
	#define ALLOCATE_MEMORY(name, size) ProfilerFn::allocateMemory(name, size)
	#define DEALLOCATE_MEMORY(name, size) ProfilerFn::deallocateMemory(name, size)
#else
	#define ENTER_PROFILE_SCOPE(name) RIO_NOOP()
	#define LEAVE_PROFILE_SCOPE() RIO_NOOP()
	#define RECORD_FLOAT(name, value) RIO_NOOP()
	#define RECORD_VECTOR3(name, value) RIO_NOOP()
	#define ALLOCATE_MEMORY(name, size) RIO_NOOP()
	#define DEALLOCATE_MEMORY(name, size) RIO_NOOP()
#endif // RIO_DEBUG
