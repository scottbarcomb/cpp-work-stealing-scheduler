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

	class GlobalThreadPool {
	public:
		explicit GlobalThreadPool(std::size_t thread_count);
		~GlobalThreadPool();

		GlobalThreadPool(const GlobalThreadPool&) = delete;
		GlobalThreadPool& operator=(const GlobalThreadPool&) = delete;

		void submit(Task task);
		void wait_for_all();

		std::size_t thread_count() const noexcept;
		std::vector<WorkerStatsSnapshot> snapshot_stats() const;

	private:
		void worker_loop(std::size_t worker_id);

		std::size_t thread_count_;
		std::unique_ptr<WorkStealingDeque> global_queue_;
		std::vector<std::thread> workers_;
		std::vector<WorkerStats> stats_;

		std::atomic<bool> shutdown_{ false };
		std::atomic<std::size_t> outstanding_tasks_{ 0 };
		std::atomic<std::size_t> submit_index_{ 0 };

		mutable std::mutex wait_mutex_;
		std::condition_variable wait_cv_;
	};

} // namespace scheduler