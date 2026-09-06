#pragma once

template<typename T>
class LockQueue
{

public:
	void Push(T job)
	{
		WRITE_LOCK;
		m_items.push(job);
	}

	T Pop()
	{
		WRITE_LOCK;
		if (m_items.empty())
			return T();

		T ret = m_items.front();
		m_items.pop();
		return ret;
	}

	void PopSize(uint64 size, OUT exvector<T>& items)
	{
		WRITE_LOCK;

		uint64 now = 0;
		while (now < size && false == m_items.empty())
		{
			items.push_back(Pop());
			++now;
		}
	}

	void PopAll(OUT exvector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
			items.push_back(item);
	}

	void Clear()
	{
		WRITE_LOCK;
		m_items = exqueue<T>();
	}

	bool Empty()
	{
		READ_LOCK;

		return m_items.empty();
	}

private:
	USE_LOCK;
	exqueue<T> m_items;
};