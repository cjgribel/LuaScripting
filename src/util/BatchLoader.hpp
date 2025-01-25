#ifndef BATCHLOADER_HPP
#define BATCHLOADER_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <vector>
#include "ThreadPool.hpp"

class BatchLoader
{
public:
    using BatchID = size_t;

    explicit BatchLoader(ThreadPool& thread_pool);

    // Queue a batch of tasks and return the batch ID
    BatchID queue_batch(const std::vector<std::function<bool()>>& tasks);

    bool did_batch_succeed(BatchID batch_id) const;

    // Check if a specific batch is complete
    bool is_batch_complete(BatchID batch_id) const;

    // Get progress of a specific batch (0.0 to 1.0)
    float get_batch_progress(BatchID batch_id) const;

private:
    struct Batch
    {
        std::atomic<size_t> total_tasks{ 0 };
        std::atomic<size_t> completed_tasks{ 0 };
        std::atomic<size_t> failed_tasks{ 0 }; // New field for failed tasks
    };

    ThreadPool& thread_pool;
    std::unordered_map<BatchID, Batch> batches;
    mutable std::mutex batches_mutex;
    std::atomic<BatchID> next_batch_id{ 1 };
};

#endif // BATCHLOADER_HPP
