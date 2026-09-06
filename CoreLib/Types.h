#pragma once

#include <mutex>
#include <atomic>
#include <string>

using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;

using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

using String = std::string;
using WString = std::wstring;

template<typename T>
using Atomic = std::atomic<T>;
using Mutex = std::mutex;
using CondVar = std::condition_variable;
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
using UniqueRef = std::unique_ptr<T>;

using RefJob = std::shared_ptr<class Job>;
using RefJobQueue = std::shared_ptr<class JobQueue>;

using TypeIndex = std::type_index;
using HashID = size_t;

#define size16(val) static_cast<int16>(sizeof(val))
#define size32(val) static_cast<int32>(sizeof(val))
#define len16(arr) static_cast<int16>(sizeof(arr) / sizeof(arr[0]))
#define len32(arr) static_cast<int32>(sizeof(arr) / sizeof(arr[0]))