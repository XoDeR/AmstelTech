#pragma once

#include "Common.h"

#include <atomic>

namespace RioGui
{

	class RIO_EXPORT Object
	{
	public:
		Object()
		{
		}

		Object(const Object&)
		{
		}

		int getRefCount() const
		{
			return refCount;
		}

		void incRef() const
		{
			++refCount;
		}

		void decRef(bool dealloc = true) const noexcept;
	protected:
		virtual ~Object();
	private:
		mutable std::atomic<int> refCount{ 0 };
	};

	// Intrusive ptr implementation
	// The advantage over C++ solutions such as std::shared_ptr is that
	// the reference count is very compactly integrated into the base object itself
	template <typename T> class IntrusivePtr
	{
	public:
		IntrusivePtr()
		{
		}

		IntrusivePtr(T* ptr)
			: ptr(ptr)
		{
			if (this->ptr)
			{
				((Object*)this->ptr)->incRef();
			}
		}

		IntrusivePtr(const IntrusivePtr& r)
			: ptr(r.ptr)
		{
			if (this->ptr)
			{
				((Object*)this->ptr)->incRef();
			}
		}

		IntrusivePtr(IntrusivePtr&& r) noexcept
			: ptr(r.ptr)
		{
			r.ptr = nullptr;
		}

		~IntrusivePtr()
		{
			if (this->ptr)
			{
				((Object*)this->ptr)->decRef();
			}
		}

		// Move another reference into the current one
		IntrusivePtr& operator=(IntrusivePtr&& r) noexcept
		{
			if (&r != this)
			{
				if (this->ptr)
				{
					((Object*)this->ptr)->decRef();
				}
				this->ptr = r.ptr;
				r.ptr = nullptr;
			}
			return *this;
		}

		// Overwrite this reference with another reference
		IntrusivePtr& operator=(const IntrusivePtr& r) noexcept
		{
			if (this->ptr != r.ptr)
			{
				if (r.ptr)
				{
					((Object*)r.ptr)->incRef();
				}
				if (this->ptr)
				{
					((Object*)this->ptr)->decRef();
				}
				this->ptr = r.ptr;
			}
			return *this;
		}

		// Overwrite this reference with a pointer to another object
		IntrusivePtr& operator=(T* ptr) noexcept
		{
			if (this->ptr != ptr)
			{
				if (ptr)
				{
					((Object*)ptr)->incRef();
				}
				if (this->ptr)
				{
					((Object*)this->ptr)->decRef();
				}
				this->ptr = ptr;
			}
			return *this;
		}

		// Compare this reference with another reference
		bool operator==(const IntrusivePtr& r) const
		{
			return this->ptr == r.ptr;
		}

		// Compare this reference with another reference
		bool operator!=(const IntrusivePtr& r) const
		{
			return this->ptr != r.ptr;
		}

		// Compare this reference with a pointer
		bool operator==(const T* ptr) const
		{
			return this->ptr == ptr;
		}

		// Compare this reference with a pointer
		bool operator!=(const T* ptr) const
		{
			return this->ptr != ptr;
		}

		// Access the object referenced by this reference
		T* operator->()
		{
			return ptr;
		}

		// Access the object referenced by this reference
		const T* operator->() const
		{
			return ptr;
		}

		// Return a C++ reference to the referenced object
		T& operator*()
		{
			return *ptr;
		}

		// Return a const C++ reference to the referenced object
		const T& operator*() const
		{
			return *ptr;
		}

		// Return a pointer to the referenced object
		operator T* ()
		{
			return ptr;
		}

		// Return a const pointer to the referenced object
		T* get()
		{
			return ptr;
		}

		// Return a pointer to the referenced object
		const T* get() const
		{
			return ptr;
		}

		// Check if the object is defined
		operator bool() const
		{
			return ptr != nullptr;
		}
	private:
		T* ptr = nullptr;
	};

} // namespace RioGui
