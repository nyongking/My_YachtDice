#pragma once

extern thread_local uint32 LThreadID;
extern thread_local uint64 LWorkerEndTick;
extern thread_local std::stack<int32> LLockStack;
extern thread_local class JobQueue* LCurrentJobQueue;