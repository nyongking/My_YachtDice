#pragma once
#include "Types.h"
#include "Allocator.h"

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <stack>
#include <array>

template<typename T>
using exvector = std::vector<T, STLAllocator<T>>;

template<typename T>
using exlist = std::list<T, STLAllocator<T>>;

template<typename T>
using exdeque = std::deque<T, STLAllocator<T>>;

template<typename T, typename Container = exdeque<T>>
using exstack = std::stack<T, Container>;

template<typename T, typename Container = exdeque<T>>
using exqueue = std::queue<T, Container>;

template<typename Key, typename T, typename Pred = std::less<Key>>
using exmap = std::map<Key, T, Pred, STLAllocator<std::pair<const Key, T>>>;

template<typename T, typename Pred = std::less<T>>
using exset = std::set<T, Pred, STLAllocator<T>>;

template<typename T, typename Container = exvector<T>, typename Pred = std::less<typename Container::value_type>>
using expriority_queue = std::priority_queue<T, Container, Pred>;

using exstring = std::basic_string<char, std::char_traits<char>, STLAllocator<char>>;
using exwstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>>;

template<typename Key, typename T, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using exhashmap = std::unordered_map<Key, T, Hasher, KeyEq, STLAllocator<std::pair<const Key, T>>>;

template<typename T, typename Hasher = std::hash<T>, typename KeyEq = std::equal_to<T>>
using exhashset = std::unordered_set<T, Hasher, KeyEq, STLAllocator<T>>;

template<typename T, uint32 size>
using exarray = std::array<T, size>;