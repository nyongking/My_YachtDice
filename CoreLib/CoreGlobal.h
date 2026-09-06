#pragma once

extern class Memory* GMemory;
extern class DeadLockProfiler* GDeadLockProfiler;
extern class ThreadManager* GThreadManager;
extern class MainThreadQueue* GMainQueue;

void InitCore();
void  ReleaseCore();

/* Job */
void PushGlobalQueue(RefJobQueue jobQueue);
void PushGlobalTimer(uint64 tickAfter, std::weak_ptr<class JobQueue> refOwner, RefJob refJob);
void DoGlobalWork(uint64 tickCount, uint64 tickWait);