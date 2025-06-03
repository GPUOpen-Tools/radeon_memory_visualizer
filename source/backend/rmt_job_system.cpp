//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a job system to run work on multiple threads.
//=============================================================================

#include "rmt_job_system.h"
#include "rmt_platform.h"
#include "rmt_assert.h"
#include <string.h>  // for memset()

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

// Job handle type
using RmtJobHandle = uint64_t;

// Definition of the job function executed by worker threads.
using JobFunction = std::function<void(int32_t thread_id, int32_t job_index, void* input)>;

// Job: A structure representing a job to be processed by the worker threads.
// It contains the function to be executed, input data and counters for tracking status.
// -------------------------------------------------------------------------------------
struct Job
{
    JobFunction          function;
    void*                input      = nullptr;
    int32_t              base_index = 0;
    int32_t              count      = 0;
    std::atomic<int32_t> run_count{0};
    std::atomic<int32_t> completed_count{0};
};

// JobQueue: A class that manages a pool of worker threads and a queue of jobs to be processed.
// It uses a condition variable to notify worker threads when new jobs are available and to signal when all jobs are done.
// -----------------------------------------------------------------------------------------------------------------------
class JobQueue
{
public:
    JobQueue(int32_t worker_thread_count);

    ~JobQueue();

    void SetAdapter(IJobQueueAdapter* wrapper);

    // Accepts a shared_ptr<Job> so the same instance is used everywhere.
    void AddJob(const std::shared_ptr<Job>& job);

    void WaitForAllJobs();

    void Shutdown();

private:
    void WorkerThreadFunc(int32_t thread_id, std::stop_token stoken);

    std::vector<std::jthread>        workers_threads_;
    std::queue<std::shared_ptr<Job>> jobs_;
    std::mutex                       queue_mutex_;
    std::condition_variable          queue_condition_;
    std::condition_variable          all_jobs_done_condition_;
    std::atomic<bool>                terminate_flag_;
    int32_t                          active_jobs_ = 0;
    IJobQueueAdapter*                wrapper_     = nullptr;
};

// Interface for the JobQueue Wrapper structure handle mapping from external APIs.
// -------------------------------------------------------------------------------
struct IJobQueueAdapter
{
public:
    virtual ~IJobQueueAdapter() = default;

    virtual std::mutex& GetHandleMutex() = 0;

    // Add a single job.
    virtual RmtErrorCode AddSingleJob(JobFunction func, void* input, RmtJobHandle* out_handle) = 0;

    // Add multiple jobs.
    virtual RmtErrorCode AddMultipleJobs(JobFunction func, void* input, int32_t base_index, int32_t count, RmtJobHandle* out_handle) = 0;

    // Wait for all jobs to complete.
    virtual void WaitForAllJobs() = 0;

    // Wait for a specific job to complete.
    virtual RmtErrorCode WaitForJobCompletion(RmtJobHandle handle) = 0;

    // Shutdown the job queue.
    virtual void Shutdown() = 0;

    // Condition variable to notify when a job is done.
    virtual void NotifyJobDone() = 0;

    // Condition variable to notify all threads all job are done.
    virtual void NotifyAllJobsDone() = 0;

    virtual std::shared_ptr<Job> GetJobByHandle(RmtJobHandle handle) = 0;
};

// JobQueue implementation: A class that manages a pool of worker threads and a queue of jobs to be processed.
// -----------------------------------------------------------------------------------------------------------

JobQueue::JobQueue(int32_t worker_thread_count)
    : terminate_flag_(false)
{
    assert(worker_thread_count > 0);
    for (int32_t i = 0; i < worker_thread_count; ++i)
    {
        workers_threads_.emplace_back([this, i](std::stop_token stop_token) { WorkerThreadFunc(i, stop_token); });
    }
}

JobQueue::~JobQueue()
{
    Shutdown();
}

void JobQueue::AddJob(const std::shared_ptr<Job>& job)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        jobs_.push(job);
    }
    queue_condition_.notify_one();
}

void JobQueue::WaitForAllJobs()
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    all_jobs_done_condition_.wait(lock, [this] { return jobs_.empty() && active_jobs_ == 0; });
}

void JobQueue::Shutdown()
{
    terminate_flag_ = true;
    queue_condition_.notify_all();
    for (auto& worker : workers_threads_)
    {
        if (worker.joinable())
        {
            worker.request_stop();
            worker.join();
        }
    }
    workers_threads_.clear();
}

void JobQueue::SetAdapter(IJobQueueAdapter* wrapper)
{
    wrapper_ = wrapper;
}

void JobQueue::WorkerThreadFunc(int32_t thread_id, std::stop_token stoken)
{
    while (!stoken.stop_requested() && !terminate_flag_)
    {
        std::shared_ptr<Job> job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this, &stoken] { return !jobs_.empty() || terminate_flag_ || stoken.stop_requested(); });
            if (terminate_flag_ || stoken.stop_requested())
                break;
            job = jobs_.front();
            jobs_.pop();
            ++active_jobs_;
        }

        for (int i = 0; i < job->count; ++i)
        {
            if (stoken.stop_requested() || terminate_flag_)
            {
                break;
            }
            job->function(thread_id, job->base_index + i, job->input);

            if (wrapper_)
            {
                ++job->completed_count;
                wrapper_->NotifyAllJobsDone();
            }
            else
            {
                ++job->completed_count;
            }
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            --active_jobs_;
            if (jobs_.empty() && active_jobs_ == 0)
            {
                all_jobs_done_condition_.notify_all();
            }
        }
    }
}

// RmtJobQueueAdapter: A wrapper around JobQueue that implements the IJobQueueAdapter interface.
// This class allows the RMT job queue API to interact with the internal JobQueue implementation.
// ----------------------------------------------------------------------------------------------

class RmtJobQueueAdapter : public IJobQueueAdapter
{
public:
    RmtJobQueueAdapter(std::unique_ptr<JobQueue> job_queue, IJobQueueAdapter* /*previous_wrapper*/)
        : job_queue(std::move(job_queue))
    {
        if (this->job_queue)
        {
            this->job_queue->SetAdapter(this);
        }
        next_handle = 1;  // Start handles from 1 to avoid confusion with invalid handle (0).
    }

    // Destructor to clean up the job queue and reset handles.
    ~RmtJobQueueAdapter() override
    {
        Shutdown();
    }

    // Factory method to create a new RMT job queue adapter.
    static IJobQueueAdapter* CreateJobQueueAdapter(std::unique_ptr<JobQueue> job_queue, IJobQueueAdapter* previous_wrapper)
    {
        return new RmtJobQueueAdapter(std::move(job_queue), previous_wrapper);
    }

    RmtErrorCode AddSingleJob(JobFunction func, void* input, RmtJobHandle* out_handle) override
    {
        return AddMultipleJobs(func, input, 0, 1, out_handle);
    }

    RmtErrorCode AddMultipleJobs(JobFunction func, void* input, int32_t base_index, int32_t count, RmtJobHandle* out_handle) override
    {
        if (!func || count <= 0)
            return kRmtErrorInvalidPointer;

        auto job        = std::make_shared<Job>();
        job->function   = func;
        job->input      = input;
        job->base_index = base_index;
        job->count      = count;

        // Generate a new handle
        RmtJobHandle handle;
        {
            std::lock_guard<std::mutex> lock(handle_mutex);
            handle                = next_handle++;
            handle_to_job[handle] = job;
        }

        // Add the job to the queue
        job_queue->AddJob(job);
        if (out_handle)
            *out_handle = handle;
        return kRmtOk;
    }

    std::mutex& GetHandleMutex() override
    {
        return handle_mutex;
    }

    virtual void NotifyJobDone() override
    {
        job_done_condition.notify_all();
    }

    // Condition variable to notify all job are done.
    virtual void NotifyAllJobsDone() override
    {
        job_done_condition.notify_all();
    }

    RmtErrorCode WaitForJobCompletion(RmtJobHandle handle) override
    {
        auto job = GetJobByHandle(handle);
        if (!job)
            return kRmtErrorInvalidPointer;

        std::unique_lock<std::mutex> lock(handle_mutex);

        job_done_condition.wait(lock, [&job] { return job->completed_count.load() >= job->count; });
        return kRmtOk;
    }

    void WaitForAllJobs() override
    {
        job_queue->WaitForAllJobs();
        std::unique_lock<std::mutex> lock(handle_mutex);
        job_done_condition.wait(lock, [this] { return handle_to_job.empty(); });
    }

    void Shutdown() override
    {
        job_queue->Shutdown();
        std::lock_guard<std::mutex> lock(handle_mutex);
        handle_to_job.clear();
        next_handle = 1;  // Reset the handle counter
    }

    // Get the job by handle
    std::shared_ptr<Job> GetJobByHandle(RmtJobHandle handle)
    {
        std::lock_guard<std::mutex> lock(handle_mutex);
        auto                        it = handle_to_job.find(handle);
        if (it != handle_to_job.end())
        {
            return it->second;
        }
        return nullptr;  // Handle not found
    }

private:
    std::unique_ptr<class JobQueue>                        job_queue;
    std::mutex                                             handle_mutex;
    RmtJobHandle                                           next_handle = 1;
    std::condition_variable                                job_done_condition;
    std::unordered_map<RmtJobHandle, std::shared_ptr<Job>> handle_to_job;
};

// Helper function to get the wrapper from the external RMT job queue structure.
inline IJobQueueAdapter* GetWrapper(RmtJobQueue* job_queue)
{
    return job_queue->wrapper;
}

// External API Functions
// ----------------------

// Initialize the job queue
RmtErrorCode RmtJobQueueInitialize(RmtJobQueue* job_queue, int32_t worker_thread_count)
{
    if (!job_queue || worker_thread_count <= 0)
        return kRmtErrorInvalidPointer;

    job_queue->wrapper = RmtJobQueueAdapter::CreateJobQueueAdapter(std::make_unique<JobQueue>(worker_thread_count), job_queue->wrapper);
    return kRmtOk;
}

// Shutdown the job queue
RmtErrorCode RmtJobQueueShutdown(RmtJobQueue* job_queue)
{
    if (!job_queue)
        return kRmtErrorInvalidPointer;

    // Get the adapter used to wrap the RmtJobQueue.
    auto* wrapper = GetWrapper(job_queue);

    wrapper->Shutdown();
    delete wrapper;

    return kRmtOk;
}

// Add a single job
RmtErrorCode RmtJobQueueAddSingle(RmtJobQueue* job_queue, RmtJobFunction func, void* input, RmtJobHandle* out_handle)
{
    return RmtJobQueueAddMultiple(job_queue, func, input, 0, 1, out_handle);
}

// Add multiple jobs
RmtErrorCode RmtJobQueueAddMultiple(RmtJobQueue* job_queue, RmtJobFunction func, void* input, int32_t base_index, int32_t count, RmtJobHandle* out_handle)
{
    if (!job_queue || !func || count <= 0)
        return kRmtErrorInvalidPointer;

    // Wrap the job queue function pointer in a std::function
    JobFunction job_func = [func](int32_t thread_id, int32_t job_index, void* input) { func(thread_id, job_index, input); };

    // Get the adapter used to wrap the RmtJobQueue.
    auto* wrapper = GetWrapper(job_queue);

    // Have the adapter create the job object
    return wrapper->AddMultipleJobs(job_func, input, base_index, count, out_handle);
}

// Wait for a job to complete
RmtErrorCode RmtJobQueueWaitForCompletion(RmtJobQueue* job_queue, RmtJobHandle handle)
{
    if (!job_queue)
        return kRmtErrorInvalidPointer;

    // Get the adapter used to wrap the RmtJobQueue.
    auto* wrapper = GetWrapper(job_queue);

    // Wait for the job to complete.
    RmtErrorCode result = wrapper->WaitForJobCompletion(handle);

    if (result != kRmtOk)
        return result;
    return kRmtOk;
}
