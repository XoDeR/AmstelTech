// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Base/Cpu.h"
#include "Core/Thread/Mutex.h"
#include "Core/Math/Uint32T.h"

// TODO replace std::list with optimized data structure
#include <list>

namespace RioCore
{
	// single - producer / single - consumer
	template <typename Ty>
	class SpScUnboundedQueueLf
	{
	private:
		struct Node
		{
			explicit Node(void* ptr)
				: m_ptr(ptr)
			{
			}

			void* m_ptr = nullptr;
			Node* m_next = nullptr;
		};

		Node* m_first = nullptr;
		Node* m_divider = nullptr;
		Node* m_last = nullptr;

	private:
		SpScUnboundedQueueLf(const SpScUnboundedQueueLf& rhs) = delete;
		SpScUnboundedQueueLf& operator=(const SpScUnboundedQueueLf& rhs) = delete;
	public:
		SpScUnboundedQueueLf()
			: m_first(new Node(nullptr))
			, m_divider(m_first)
			, m_last(m_first)
		{
		}

		~SpScUnboundedQueueLf()
		{
			while (nullptr != m_first)
			{
				Node* node = m_first;
				m_first = node->m_next;
				delete node;
			}
		}

		void push(Ty* _ptr) // producer only
		{
			m_last->m_next = new Node((void*)_ptr);
			atomicExchangePtr((void**)&m_last, m_last->m_next);
			while (m_first != m_divider)
			{
				Node* node = m_first;
				m_first = m_first->m_next;
				delete node;
			}
		}

		Ty* peek() // consumer only
		{
			if (m_divider != m_last)
			{
				Ty* ptr = (Ty*)m_divider->m_next->m_ptr;
				return ptr;
			}

			return nullptr;
		}

		Ty* pop() // consumer only
		{
			if (m_divider != m_last)
			{
				Ty* ptr = (Ty*)m_divider->m_next->m_ptr;
				atomicExchangePtr((void**)&m_divider, m_divider->m_next);
				return ptr;
			}

			return nullptr;
		}


	};

#if RIO_CONFIG_SUPPORTS_THREADING
	template<typename Ty>
	class SpScUnboundedQueueMutex
	{
	private:
		SpScUnboundedQueueMutex(const SpScUnboundedQueueMutex& rhs) = delete;
		SpScUnboundedQueueMutex& operator=(const SpScUnboundedQueueMutex& rhs) = delete;
	public:
		SpScUnboundedQueueMutex()
		{
		}

		~SpScUnboundedQueueMutex()
		{
			RIO_CHECK(m_queue.empty(), "Queue is not empty!");
		}

		void push(Ty* _item)
		{
			RioCore::ScopedMutex lock(m_mutex);
			m_queue.push_back(_item);
		}

		Ty* peek()
		{
			RioCore::ScopedMutex lock(m_mutex);
			if (!m_queue.empty())
			{
				return m_queue.front();
			}

			return nullptr;
		}

		Ty* pop()
		{
			RioCore::ScopedMutex lock(m_mutex);
			if (!m_queue.empty())
			{
				Ty* item = m_queue.front();
				m_queue.pop_front();
				return item;
			}

			return nullptr;
		}

	private:
		RioCore::Mutex m_mutex;
		std::list<Ty*> m_queue;
	};
#endif // RIO_CONFIG_SUPPORTS_THREADING

#if RIO_CONFIG_SPSCQUEUE_USE_MUTEX && RIO_CONFIG_SUPPORTS_THREADING
#define SpScUnboundedQueue SpScUnboundedQueueMutex
#else
#define SpScUnboundedQueue SpScUnboundedQueueLf
#endif // RIO_CONFIG_SPSCQUEUE_USE_MUTEX

#if RIO_CONFIG_SUPPORTS_THREADING
	template <typename Ty>
	class SpScBlockingUnboundedQueue
	{
	private:
		SpScBlockingUnboundedQueue(const SpScBlockingUnboundedQueue& rhs) = delete;
		SpScBlockingUnboundedQueue& operator=(const SpScBlockingUnboundedQueue& rhs) = delete;
	public:
		SpScBlockingUnboundedQueue()
		{
		}

		~SpScBlockingUnboundedQueue()
		{
		}

		void push(Ty* _ptr) // producer only
		{
			m_queue.push((void*)_ptr);
			m_count.post();
		}

		Ty* peek() // consumer only
		{
			return (Ty*)m_queue.peek();
		}

		Ty* pop(int32_t _msecs = -1) // consumer only
		{
			if (m_count.wait(_msecs))
			{
				return (Ty*)m_queue.pop();
			}

			return NULL;
		}

	private:
		Semaphore m_count;
		SpScUnboundedQueue<void> m_queue;
	};
#endif // RIO_CONFIG_SUPPORTS_THREADING

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka