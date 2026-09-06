#pragma once
#include <unordered_map>
#include <functional>

/*---------------------------------------
	ThreadSafeHashMap<K, V>
	- std::unordered_map을 기존 RW 락으로 감싼 스레드 안전 해시맵
	- 읽기 연산: READ_LOCK  (다수 동시 읽기 허용)
	- 쓰기 연산: WRITE_LOCK (단독 쓰기)
---------------------------------------*/

template<typename K, typename V>
class ThreadSafeHashMap
{
public:
	ThreadSafeHashMap() = default;
	~ThreadSafeHashMap() = default;

	// 복사 금지
	ThreadSafeHashMap(const ThreadSafeHashMap&) = delete;
	ThreadSafeHashMap& operator=(const ThreadSafeHashMap&) = delete;

public:
	/* ---- 읽기 연산 (READ_LOCK) ---- */

	// key에 해당하는 값을 outValue에 복사. 존재하면 true 반환
	bool Find(const K& key, V& outValue) const
	{
		READ_LOCK;
		auto it = m_map.find(key);
		if (it == m_map.end())
			return false;
		outValue = it->second;
		return true;
	}

	// 포인터 반환 (호출자가 외부 락을 보유해야 함). 성능 임계 경로용
	V* FindUnsafe(const K& key)
	{
		auto it = m_map.find(key);
		if (it == m_map.end())
			return nullptr;
		return &it->second;
	}

	// key 존재 여부 확인
	bool Contains(const K& key) const
	{
		READ_LOCK;
		return m_map.find(key) != m_map.end();
	}

	// 현재 원소 수
	size_t Size() const
	{
		READ_LOCK;
		return m_map.size();
	}

	// 비어 있는지 확인
	bool Empty() const
	{
		READ_LOCK;
		return m_map.empty();
	}

	// 읽기 락 하에 전체 순회
	void ForEachRead(std::function<void(const K&, const V&)> func) const
	{
		READ_LOCK;
		for (const auto& pair : m_map)
			func(pair.first, pair.second);
	}

	/* ---- 쓰기 연산 (WRITE_LOCK) ---- */

	// key가 없을 때만 삽입. 삽입했으면 true 반환
	bool Insert(const K& key, const V& value)
	{
		WRITE_LOCK;
		auto result = m_map.emplace(key, value);
		return result.second;
	}

	// key가 없으면 삽입, 있으면 덮어씀
	bool InsertOrAssign(const K& key, V value)
	{
		WRITE_LOCK;
		m_map.insert_or_assign(key, std::move(value));
		return true;
	}

	// std::unordered_map과 동일 의미 (없으면 기본값 생성)
	V& operator[](const K& key)
	{
		WRITE_LOCK;
		return m_map[key];
	}

	// key 제거. 제거했으면 true 반환
	bool Erase(const K& key)
	{
		WRITE_LOCK;
		return m_map.erase(key) > 0;
	}

	// 전체 삭제
	void Clear()
	{
		WRITE_LOCK;
		m_map.clear();
	}

	// 쓰기 락 하에 전체 순회
	void ForEach(std::function<void(const K&, V&)> func)
	{
		WRITE_LOCK;
		for (auto& pair : m_map)
			func(pair.first, pair.second);
	}

private:
	USE_LOCK;
	std::unordered_map<K, V> m_map;
};
