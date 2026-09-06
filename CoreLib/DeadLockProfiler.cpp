#include "CorePch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(m_lock);

	int32	lockID = 0;
	auto iter = m_nameTolD.find(name);
	if (iter == m_nameTolD.end())
	{
		lockID = static_cast<int32>(m_nameTolD.size());

		m_nameTolD.emplace(name, lockID);
		m_IDToName.emplace(lockID, name);
	}
	else
	{
		lockID = iter->second;
	}

	// 잡고 있는 락인가?
	if (false == LLockStack.empty())
	{
		const int32 prevID = LLockStack.top();
		if (lockID != prevID)
		{
			std::set<int32>& history = m_lockHistory[prevID];
			if (history.find(lockID) == history.end()) // 처음 발견한 lock
			{
				history.insert(lockID);
				CheckCycle();
			}
		}
	}

	LLockStack.push(lockID);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(m_lock);

	if (LLockStack.empty())
		TRIGGER_CRASH("INVALID_UNLOCK");

	int32 lockID = m_nameTolD[name];
	if (lockID != LLockStack.top())
		TRIGGER_CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(m_nameTolD.size());
	m_discoverOrder = std::vector<int32>(lockCount, -1); // 초기상태로 비운다.
	m_discoveredCount = 0;
	m_finished = std::vector<bool>(lockCount, false); // 초기상태로 비운다.
	m_parent = std::vector<int32>(lockCount, -1);// 초기상태로 비운다.

	for (int32 lockID = 0; lockID < lockCount; ++lockID)
		Dfs(lockID);

	m_discoverOrder.clear();
	m_finished.clear();
	m_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (-1 != m_discoverOrder[here]) // 이미 발견된 index
		return;

	m_discoverOrder[here] = m_discoveredCount++;

	auto iter = m_lockHistory.find(here);

	// 인접 노드가 존재하지 않은 경우
	if (iter == m_lockHistory.end())
	{
		m_finished[here] = true;
		return;
	}

	// 인접 노드를 순회한다.
	std::set<int32>& nextSet = iter->second;
	for (int32 there : nextSet)
	{
		// 미방문노드의 처리
		if (-1 == m_discoverOrder[there])
		{
			m_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 순방향 (there의 발견보다 here의 발견이 더 먼저이다)
		if (m_discoverOrder[here] < m_discoverOrder[there])
			continue;

		// 역방향 (there의 dfs가 종료되지 않았는데, there는 here의 선조이다)
		if (false == m_finished[there])
		{
			printf("%s -> %s\n", m_IDToName[here], m_IDToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", m_IDToName[m_parent[now]], m_IDToName[now]);
				now = m_parent[now];
				if (now == there)
					break;
			}

			TRIGGER_CRASH("DEADLOCK");
		}
	}

	m_finished[here] = true;

}
