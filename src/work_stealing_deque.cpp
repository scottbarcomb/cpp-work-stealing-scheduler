#pragma once
#include "../include/scheduler/work_stealing_deque.hpp"

namespace scheduler {

    void WorkStealingDeque::push(Task task) {
        std::lock_guard<std::mutex> lock(mutex_);
        deque_.push_back(std::move(task));
    }

    std::optional<Task> WorkStealingDeque::pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (deque_.empty())
            return std::nullopt;
        Task task = std::move(deque_.front());
        deque_.pop_front();
        return std::optional<Task>(std::move(task));
    }

    std::optional<Task> WorkStealingDeque::steal() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (deque_.empty())
            return std::nullopt;
        Task task = std::move(deque_.back());
        deque_.pop_back();
        return std::optional<Task>(std::move(task));
    }

    bool WorkStealingDeque::empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.empty();
    }

    std::size_t WorkStealingDeque::size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.size();
    }

} // namespace scheduler
