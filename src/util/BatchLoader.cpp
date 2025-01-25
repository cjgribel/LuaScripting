#include "BatchLoader.hpp"

BatchLoader::BatchLoader(ThreadPool& thread_pool)
    : thread_pool(thread_pool)
{
}

BatchLoader::BatchID BatchLoader::queue_batch(const std::vector<std::function<bool()>>& tasks)
{
    // Generate a new batch ID
    BatchID batch_id = next_batch_id++;

    // Lock once and initialize the batch
    {
        std::lock_guard<std::mutex> lock(batches_mutex);
        auto& batch = batches[batch_id];
        batch.total_tasks = tasks.size();
    }

    // Queue each task in the batch
    for (const auto& task : tasks)
    {
        thread_pool.queue_task([this, batch_id, task]()
            {
                // Execute the task and determine success or failure
                bool success = task();

                {
                    std::lock_guard<std::mutex> lock(batches_mutex);
                    auto it = batches.find(batch_id);
                    if (it != batches.end())
                    {
                        if (success)
                            it->second.completed_tasks++;
                        else
                            it->second.failed_tasks++;
                    }
                }
            });
    }

    return batch_id;
}

bool BatchLoader::did_batch_succeed(BatchID batch_id) const
{
    std::lock_guard<std::mutex> lock(batches_mutex);
    auto it = batches.find(batch_id);
    if (it != batches.end())
    {
        // A batch succeeds if no tasks failed and all tasks are completed
        return it->second.failed_tasks == 0 && 
               it->second.completed_tasks == it->second.total_tasks;
    }
    return false; // Batch does not exist
}

bool BatchLoader::is_batch_complete(BatchID batch_id) const
{
    std::lock_guard<std::mutex> lock(batches_mutex);
    auto it = batches.find(batch_id);
    if (it != batches.end())
    {
        const Batch& batch = it->second;
        return (batch.completed_tasks + batch.failed_tasks) == batch.total_tasks;
    }
    return true; // Batch does not exist, treat as complete
}

float BatchLoader::get_batch_progress(BatchID batch_id) const
{
    std::lock_guard<std::mutex> lock(batches_mutex);
    auto it = batches.find(batch_id);
    if (it != batches.end() && it->second.total_tasks > 0)
    {
        const Batch& batch = it->second;
        // Progress includes both successful and failed tasks
        size_t processed_tasks = batch.completed_tasks + batch.failed_tasks;
        return static_cast<float>(processed_tasks) / batch.total_tasks;
    }
    return 1.0f; // Batch does not exist, treat as fully complete
}
