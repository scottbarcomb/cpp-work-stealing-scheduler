#pragma once
#include "../include/scheduler/thread_pool.hpp"

namespace scheduler {

	ThreadPool::ThreadPool(std::size_t thread_count) : thread_count_(thread_count), queues_(thread_count), stats_(thread_count) {
		if (thread_count_ == 0)
			throw std::invalid_argument("ThreadPool must contain at least one thread.");

		for (std::size_t i = 0; i < thread_count_; i++)
			queues_.at(i) = std::make_unique<WorkStealingDeque>();

		workers_.reserve(thread_count_);
		for (std::size_t i = 0; i < thread_count_; i++)
			workers_.emplace_back([this, i] {worker_loop(i); });
	}

	ThreadPool::~ThreadPool() {
		shutdown_.store(true, std::memory_order_release);
		wait_cv_.notify_all();

		// Destroy threads
		for (auto& worker : workers_)
			if (worker.joinable())
				worker.join();
	}

	void ThreadPool::submit(Task task) {
		// Increment task list
		outstanding_tasks_.fetch_add(1, std::memory_order_acq_rel);

		// Choose a worker queue using round robin
		const std::size_t index = submit_index_.fetch_add(1, std::memory_order_relaxed) % thread_count_;

		// Push task into that queue
		try { queues_.at(index)->push(std::move(task)); }
		catch (std::exception) {
			outstanding_tasks_.fetch_sub(1, std::memory_order_acq_rel);
			throw;
		}
	}

	// Debug function to test work stealing functionality
	void ThreadPool::forceSubmitOneThread(Task task) {
		outstanding_tasks_.fetch_add(1, std::memory_order_acq_rel);
		const std::size_t index = 0;
		try { queues_.at(index)->push(std::move(task)); }
		catch (std::exception) {
			outstanding_tasks_.fetch_sub(1, std::memory_order_acq_rel);
			throw;
		}
	}

	void ThreadPool::wait_for_all() {
		std::unique_lock lock(wait_mutex_);
		wait_cv_.wait(lock, [this] { return outstanding_tasks_.load(std::memory_order_acquire) == 0; });
	}

	std::size_t ThreadPool::thread_count() const noexcept {
		return thread_count_;
	}

	std::vector<WorkerStatsSnapshot> ThreadPool::snapshot_stats() const {
		std::vector<WorkerStatsSnapshot> result;
		result.reserve(thread_count_);

		for (const auto& stat : stats_) {
			result.push_back(WorkerStatsSnapshot{
				.executed_tasks = stat.executed_tasks.load(std::memory_order_relaxed),
				.successful_steals = stat.successful_steals.load(std::memory_order_relaxed),
				.failed_steals = stat.failed_steals.load(std::memory_order_relaxed),
				.idle_loops = stat.idle_loops.load(std::memory_order_relaxed)
			});
		}

		return result;
	}

	void ThreadPool::worker_loop(std::size_t worker_id) {
		while (!shutdown_.load(std::memory_order_acquire)) {
			std::optional<Task> task = queues_.at(worker_id)->pop();

			if (!task)
				task = steal_task(worker_id);

			if (task) {
				// Execute
				try { (*task)(); }
				catch (std::exception) { throw; }

				stats_.at(worker_id).executed_tasks.fetch_add(1, std::memory_order_relaxed);

				if (outstanding_tasks_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
					std::scoped_lock lock(wait_mutex_);
					wait_cv_.notify_all();
				}
			}
			else {
				stats_.at(worker_id).idle_loops.fetch_add(1, std::memory_order_relaxed);
				std::this_thread::yield();
			}
		}
	}

	std::optional<Task> ThreadPool::steal_task(std::size_t thief_id) {
		for (size_t offset = 1; offset < thread_count_; offset++) {
			const std::size_t victim_id = (thief_id + offset) % thread_count_;

			if (auto task = queues_.at(victim_id)->steal()) {
				stats_.at(thief_id).successful_steals.fetch_add(1, std::memory_order_relaxed);
				return task;
			}
		}

		stats_.at(thief_id).failed_steals.fetch_add(1, std::memory_order_relaxed);
		return std::nullopt;
	}

} // namespace scheduler