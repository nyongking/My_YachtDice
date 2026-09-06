#pragma once

#include <functional>

struct IJobDispatcher
{
    virtual ~IJobDispatcher() = default;
    virtual void ParallelFor(int start, int end, std::function<void(int)> body) = 0;
};

struct SingleThreadDispatcher : IJobDispatcher
{
    void ParallelFor(int start, int end, std::function<void(int)> body) override
    {
        for (int i = start; i < end; ++i)
            body(i);
    }
};
