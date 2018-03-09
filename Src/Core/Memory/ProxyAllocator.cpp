#include "Core/Error/Error.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Profiler.h"

namespace Rio
{

ProxyAllocator::ProxyAllocator(Allocator& allocator, const char* name)
	: allocator(allocator)
	, name(name)
{
	RIO_ASSERT(name != nullptr, "Name must be != nullptr");
}

void* ProxyAllocator::allocate(uint32_t size, uint32_t align)
{
	void* p = this->allocator.allocate(size, align);
	ALLOCATE_MEMORY(this->name, this->allocator.getAllocatedSize(p));
	return p;
}

void ProxyAllocator::deallocate(void* data)
{
	DEALLOCATE_MEMORY(this->name, (data == nullptr) ? 0 : this->allocator.getAllocatedSize((const void*)data));
	this->allocator.deallocate(data);
}

const char* ProxyAllocator::getProxyAllocatorName() const
{
	return this->name;
}

} // namespace Rio
