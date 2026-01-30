#pragma once

#include <queue>
#include <utility>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafetyQueue {
private:
    bool isQueueClosed;
    std::queue<T> queue;
    std::condition_variable cv;
    std::mutex mtx;
    
public:
    ThreadSafetyQueue() : isQueueClosed(false) {
    }
    
    ThreadSafetyQueue(const ThreadSafetyQueue<T> &) = delete;
    ThreadSafetyQueue(ThreadSafetyQueue<T> &&) = delete;
    ThreadSafetyQueue& operator=(const ThreadSafetyQueue<T> &) = delete;
    ThreadSafetyQueue& operator=(ThreadSafetyQueue<T> &&) = delete;

    void pushToQueue(T &&cmd) {
        std::unique_lock<std::mutex> uLock(mtx);
        queue.push(std::move(cmd));
        cv.notify_one();
    }
    void close() {
        std::unique_lock<std::mutex> uLock(mtx);
        isQueueClosed = true;
        cv.notify_all();
    }
    bool pop(T &cmd) {
        std::unique_lock<std::mutex> uLock(mtx);
        
        cv.wait(uLock, [&]{ return isQueueClosed || !queue.empty(); });
        if (queue.empty()) {
            return false;
        }
        cmd = std::move(queue.front());
        queue.pop();

        return true;
    }

    ~ThreadSafetyQueue() {
        if (!isQueueClosed) {
            close();
        }
    }
};