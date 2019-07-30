#pragma once

#include <execution>
#include <thread>
#include <future>
#include <cmath>

template <typename RandomIt, typename T, typename PredicateT, typename CompareT = std::less<>>
RandomIt filterMinElement(std::execution::parallel_unsequenced_policy, RandomIt begin, RandomIt end, T init, PredicateT&& p, CompareT&& cmp = CompareT{})
{
    // an experimental implementation using asyc
    // does good on msvc's thread pools

    constexpr int minParallelBatchSize = 256 * 128;
    const int size = static_cast<int>(std::distance(begin, end));
    const int numThreads = std::clamp(size / minParallelBatchSize, 1, static_cast<int>(std::thread::hardware_concurrency()));

    auto makeWorker = [&](RandomIt begin, RandomIt end) {
        return [init, begin, end, p, cmp] () mutable {
            RandomIt ret = begin;
            while (begin != end)
            {
                if (p(*begin))
                {
                    if (cmp(*begin, init))
                    {
                        init = *begin;
                        ret = begin;
                    }
                }
                ++begin;
            }
            return ret;
        };
    };

    std::vector<std::future<RandomIt>> partialMins;
    partialMins.reserve(numThreads);
    const int batchSize = size / numThreads;
    for (int i = 0; i < numThreads-1; ++i)
    {
        auto b = begin + batchSize * i;
        auto e = begin + batchSize * (i + 1);
        partialMins.emplace_back(std::async(std::launch::async, makeWorker(b, e)));
    }

    {
        // last batch in this thread
        auto b = begin + batchSize * (numThreads - 1);
        auto e = end;
        std::promise<RandomIt> pr;
        pr.set_value(makeWorker(b, e)());
        partialMins.emplace_back(pr.get_future());
    }

    RandomIt minIt = end;
    for (auto& fut : partialMins)
    {
        auto it = fut.get();

        if (p(*it))
        {
            if (cmp(*it, init))
            {
                init = *it;
                minIt = it;
            }
        }
    }

    return minIt;
}
