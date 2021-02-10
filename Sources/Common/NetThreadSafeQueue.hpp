#ifndef NETTHREADSAFEQUEUE_HPP
#define NETTHREADSAFEQUEUE_HPP
#include "NetCommon.hpp"

namespace net
{
	template<typename T>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue() = default;
		ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
		virtual ~ThreadSafeQueue() { Clear(); }

	public:
		const T& Front()
		{
			std::scoped_lock lock(m_MutexQueue);
			return m_DeQueue.front();
		}

		const T& Back()
		{
			std::scoped_lock lock(m_MutexQueue);
			return m_DeQueue.back();
		}

		void PushBack(const T& item)
		{
			std::scoped_lock lock(m_MutexQueue);
			m_DeQueue.emplace_back(std::move(item));
		}

		void PushFront(const T& item)
		{
			std::scoped_lock lock(m_MutexQueue);
			m_DeQueue.emplace_front(std::move(item));
		}

		bool Empty()
		{
			std::scoped_lock lock(m_MutexQueue);
			return m_DeQueue.empty();
		}

		size_t Count()
		{
			std::scoped_lock lock(m_MutexQueue);
			m_DeQueue.clear();
		}

		void Clear()
		{
			std::scoped_lock lock(m_MutexQueue);
			m_DeQueue.empty();
		}

		T PopFront()
		{
			std::scoped_lock lock(m_MutexQueue);
			auto t = std::move(m_DeQueue.front());
			m_DeQueue.pop_front();
			return t;
		}

		T PopBack()
		{
			std::scoped_lock lock(m_MutexQueue);
			auto t = std::move(m_DeQueue.back());
			m_DeQueue.pop_back();
			return t;
		}

	private:
		std::mutex m_MutexQueue;
		std::deque<T> m_DeQueue;
	};
}

#endif