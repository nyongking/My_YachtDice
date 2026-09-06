#pragma once
#include <stack>
#include <vector>
#include <map>

/* -----------------
DeadLock Detection
----------------- */

class  DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 here);

private:
	Mutex m_lock;

	std::unordered_map<const char*, int32>	m_nameTolD;
	std::unordered_map<int32, const char*>	m_IDToName;
	std::map<int32, std::set<int32>>				m_lockHistory;


private:
	int32									m_discoveredCount = 0;
	std::vector<int32>						m_discoverOrder;
	std::vector<bool>						m_finished;
	std::vector<int32>						m_parent;
};

