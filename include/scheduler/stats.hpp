#pragma once
#include <atomic>
#include <cstddef>

namespace scheduler {

	struct WorkerStats {
		std::atomic<std::size_t> executed_tasks{ 0 };
		std::atomic<std::size_t> successful_steals{ 0 };
		std::atomic<std::size_t> failed_steals{ 0 };
		std::atomic<std::size_t> idle_loops{ 0 };
	};

	// Cannot return atomics by copying the above struct when needed, so load them into this struct
	struct WorkerStatsSnapshot {
		std::size_t executed_tasks;
		std::size_t successful_steals;
		std::size_t failed_steals;
		std::size_t idle_loops;
	};

} // namespace scheduler