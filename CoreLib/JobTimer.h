#pragma once

struct JobData
{
	JobData(std::weak_ptr<JobQueue> owner, RefJob refJob) : m_owner(owner), m_refJob(refJob) {}

	std::weak_ptr<JobQueue> m_owner;
	RefJob			   m_refJob;
};

struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		return m_ExecuteTick > other.m_ExecuteTick;
	}

	uint64 m_ExecuteTick = 0;
	JobData* m_pJobData = nullptr;
};

class JobTimer
{
public:
	void Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, RefJob refJob);
	void Distribute(uint64 now);
	void Clear();
	bool HasWork();

private:
	USE_LOCK;
	expriority_queue<TimerItem>		m_Items;
	Atomic<bool>					m_distrubuting = false;
};

