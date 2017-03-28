#pragma once

#include "SpscQueue.h"

namespace RioCore
{
	template <typename Ty>
	class MpScUnboundedQueue
	{
	private:
		Mutex m_write;
		SpScUnboundedQueue<Ty> m_queue;
	private:
		MpScUnboundedQueue(const MpScUnboundedQueue& rhs) = delete;
		MpScUnboundedQueue& operator=(const MpScUnboundedQueue& rhs) = delete;
	public:
		MpScUnboundedQueue()
		{
		}

		~MpScUnboundedQueue()
		{
		}

		void push(Ty* ptr) // producer only
		{
			ScopedMutex lock(m_write);
			m_queue.push(ptr);
		}

		Ty* peek() // consumer only
		{
			return m_queue.peek();
		}

		Ty* pop() // consumer only
		{
			return m_queue.pop();
		}
	};

	template <typename Ty>
	class MpScUnboundedBlockingQueue
	{
	private:
		MpScUnboundedQueue<Ty> m_queue;
		Semaphore m_sem;
	private:
		MpScUnboundedBlockingQueue(const MpScUnboundedBlockingQueue& rhs) = delete;
		MpScUnboundedBlockingQueue& operator=(const MpScUnboundedBlockingQueue& rhs) = delete;
	public:
		MpScUnboundedBlockingQueue()
		{
		}

		~MpScUnboundedBlockingQueue()
		{
		}

		void push(Ty* ptr) // producer only
		{
			m_queue.push(ptr);
			m_sem.post();
		}

		Ty* pop() // consumer only
		{
			m_sem.wait();
			return m_queue.pop();
		}
	};

} // namespace RioCore
