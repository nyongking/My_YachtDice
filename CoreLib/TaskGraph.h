#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Job.h"
#include "CountdownEvent.h"

/*---------------------------------------
	TaskNode — TaskGraph 내부 작업 노드
	(직접 생성하지 말고 TaskGraph::AddTask() 사용)
---------------------------------------*/
class TaskNode
{
	friend class TaskGraph;

public:
	// 이 노드가 other 노드에 의존함을 선언
	// (other가 완료된 후에 이 노드가 실행됨)
	TaskNode* DependsOn(TaskNode* other);

	int32 GetId() const { return m_id; }
	const std::string& GetName() const { return m_name; }

private:
	TaskNode(int32 id, const std::string& name, CallbackType callback)
		: m_id(id)
		, m_name(name)
		, m_callback(std::move(callback))
		, m_unfinishedDependencies(0)
	{
	}

private:
	int32					m_id;
	std::string				m_name;
	CallbackType			m_callback;

	std::vector<int32>		m_dependencies;		// 이 노드가 의존하는 노드 ID 목록
	std::vector<int32>		m_dependents;		// 이 노드에 의존하는 노드 ID 목록

	Atomic<int32>			m_unfinishedDependencies;
};

/*---------------------------------------
	TaskGraph
	- DAG(방향 비순환 그래프) 기반 작업 의존성 시스템
	- 의존 관계를 선언하면 최대 병렬도를 유지하며 순서대로 실행
	- 내부적으로 CountdownEvent + JobQueue 사용
---------------------------------------*/
class TaskGraph
{
public:
	TaskGraph() = default;
	~TaskGraph() = default;

	// 복사 금지
	TaskGraph(const TaskGraph&) = delete;
	TaskGraph& operator=(const TaskGraph&) = delete;

public:
	// 태스크 추가. 반환된 포인터로 DependsOn 체이닝 가능
	TaskNode* AddTask(const std::string& name, CallbackType callback);

	// taskId 노드가 dependencyId 노드에 의존함을 선언
	void DependsOn(int32 taskId, int32 dependencyId);

	// 그래프 실행 — 모든 태스크가 완료될 때까지 블로킹
	void Execute();

	// 모든 노드 제거 (재사용 가능)
	void Clear();

	// 사이클 검사. 사이클이 없으면 true 반환
	bool Validate() const;

private:
	TaskNode* FindNode(int32 id) const;
	void SubmitReady(TaskNode* node, class CountdownEvent* countdown);

private:
	std::vector<std::unique_ptr<TaskNode>>	m_nodes;
	int32									m_nextId = 0;
};
