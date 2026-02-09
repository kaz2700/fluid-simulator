#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

// Simple thread pool for parallel task execution
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
        : stop(false), activeTasks(0) {
        if (numThreads == 0) numThreads = 4; // Default to 4 threads if hardware_concurrency fails
        
        workers.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                        activeTasks++;
                    }
                    
                    task();
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        activeTasks--;
                        finishedCondition.notify_one();
                    }
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    // Add a task to the pool
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        
        condition.notify_one();
        return res;
    }
    
    // Wait for all tasks to complete
    void waitAll() {
        std::unique_lock<std::mutex> lock(queueMutex);
        finishedCondition.wait(lock, [this] { return tasks.empty() && activeTasks == 0; });
    }
    
    // Get number of threads
    size_t size() const {
        return workers.size();
    }
    
    // Parallel for loop - distribute work across threads
    template<typename Func>
    void parallelFor(size_t start, size_t end, Func&& func) {
        size_t numThreads = workers.size();
        size_t totalWork = end - start;
        
        if (totalWork == 0) return;
        
        // If work is small or only 1 thread, execute sequentially
        if (totalWork < numThreads * 4 || numThreads == 1) {
            for (size_t i = start; i < end; ++i) {
                func(i);
            }
            return;
        }
        
        size_t chunkSize = totalWork / numThreads;
        std::vector<std::future<void>> futures;
        futures.reserve(numThreads);
        
        for (size_t t = 0; t < numThreads; ++t) {
            size_t chunkStart = start + t * chunkSize;
            size_t chunkEnd = (t == numThreads - 1) ? end : chunkStart + chunkSize;
            
            futures.push_back(enqueue([chunkStart, chunkEnd, &func]() {
                for (size_t i = chunkStart; i < chunkEnd; ++i) {
                    func(i);
                }
            }));
        }
        
        // Wait for all chunks to complete
        for (auto& f : futures) {
            f.get();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable finishedCondition;
    
    std::atomic<bool> stop;
    std::atomic<size_t> activeTasks;
};

#endif // THREAD_POOL_HPP
