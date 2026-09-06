#include "CorePch.h"
#include "TaskGraph.h"
#include "Job.h"
#include "JobQueue.h"
#include "CoreGlobal.h"

#include <unordered_set>
#include <stack>

/* -----------------------------------------------------------------------
	TaskNode
----------------------------------------------------------------------- */

TaskNode* TaskNode::DependsOn(TaskNode* other)
{
	ASSERT_TRIG_CRASH(other != nullptr);

	m_dependencies.push_back(other->m_id);
	other->m_dependents.push_back(m_id);
	return this;
}

/* -----------------------------------------------------------------------
	TaskGraph
----------------------------------------------------------------------- */

TaskNode* TaskGraph::AddTask(const std::string& name, CallbackType callback)
{
	int32 id = m_nextId++;
	// TaskNode 생성자가 private이므로 new + unique_ptr 사용
	m_nodes.push_back(std::unique_ptr<TaskNode>(new TaskNode(id, name, std::move(callback))));
	return m_nodes.back().get();
}

void TaskGraph::DependsOn(int32 taskId, int32 dependencyId)
{
	TaskNode* task = FindNode(taskId);
	TaskNode* dependency = FindNode(dependencyId);

	ASSERT_TRIG_CRASH(task != nullptr);
	ASSERT_TRIG_CRASH(dependency != nullptr);

	task->m_dependencies.push_back(dependencyId);
	dependency->m_dependents.push_back(taskId);
}

void TaskGraph::Execute()
{
	if (m_nodes.empty())
		return;

	ASSERT_TRIG_CRASH(Validate());

	const int32 totalCount = static_cast<int32>(m_nodes.size());

	// 각 노드의 미완료 의존성 카운터 초기화
	for (auto& node : m_nodes)
	{
		node->m_unfinishedDependencies.store(
			static_cast<int32>(node->m_dependencies.size())
		);
	}

	// 모든 노드 완료를 기다리는 이벤트
	CountdownEvent countdown(totalCount);

	// 루트 노드(의존성 없는 노드)를 먼저 제출
	for (auto& node : m_nodes)
	{
		if (node->m_unfinishedDependencies.load() == 0)
		{
			SubmitReady(node.get(), &countdown);
		}
	}

	countdown.Wait();
}

void TaskGraph::Clear()
{
	m_nodes.clear();
	m_nextId = 0;
}

bool TaskGraph::Validate() const
{
	// DFS 기반 사이클 탐지
	// 상태: 0=미방문, 1=방문중(스택에 있음), 2=방문완료
	std::vector<int32> state(m_nextId, 0);

	std::function<bool(int32)> hasCycle = [&](int32 id) -> bool
	{
		state[id] = 1;

		TaskNode* node = FindNode(id);
		if (node == nullptr)
			return false;

		for (int32 depId : node->m_dependents)
		{
			if (state[depId] == 1)
				return true;	// 사이클 발견
			if (state[depId] == 0 && hasCycle(depId))
				return true;
		}

		state[id] = 2;
		return false;
	};

	for (auto& node : m_nodes)
	{
		if (state[node->m_id] == 0)
		{
			if (hasCycle(node->m_id))
				return false;
		}
	}

	return true;
}

TaskNode* TaskGraph::FindNode(int32 id) const
{
	for (auto& node : m_nodes)
	{
		if (node->m_id == id)
			return node.get();
	}
	return nullptr;
}

void TaskGraph::SubmitReady(TaskNode* node, CountdownEvent* countdown)
{
	TaskGraph* graph = this;

	auto queue = std::make_shared<JobQueue>();
	queue->DoAsync(
		/*pushOnly=*/true,
		[node, countdown, graph]()
		{
			// 실제 작업 실행
			node->m_callback();

			// 이 노드에 의존하는 노드들의 카운터 감소
			for (int32 depId : node->m_dependents)
			{
				TaskNode* dependent = graph->FindNode(depId);
				if (dependent == nullptr)
					continue;

				const int32 remaining = dependent->m_unfinishedDependencies.fetch_sub(1) - 1;
				if (remaining == 0)
				{
					// 모든 의존성 완료 → 이 노드도 제출 가능
					graph->SubmitReady(dependent, countdown);
				}
			}

			// 이 노드 완료 신호
			countdown->Signal();
		}
	);
}
