#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>

#include "task.hpp"
#include "work_stealing_deque.hpp"
#include "stats.hpp"

namespace scheduler {

	class ThreadPool {
	public:
		explicit ThreadPool(std::size_t thread_count);
		~ThreadPool();

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		void submit(Task task);
		void forceSubmitOneThread(Task task); // To test work stealing
		void wait_for_all();

		std::size_t thread_count() const noexcept;
		std::vector<WorkerStatsSnapshot> snapshot_stats() const;

	private:
		void worker_loop(std::size_t worker_id);
		std::optional<Task> steal_task(std::size_t thief_id);

		std::size_t thread_count_;
		std::vector<std::unique_ptr<WorkStealingDeque>> queues_;
		std::vector<std::thread> workers_;
		std::vector<WorkerStats> stats_;

		std::atomic<bool> shutdown_{ false };
		std::atomic<std::size_t> outstanding_tasks_{ 0 };
		std::atomic<std::size_t> submit_index_{ 0 };

		mutable std::mutex wait_mutex_;
		std::condition_variable wait_cv_;
	};

} // namespace scheduler