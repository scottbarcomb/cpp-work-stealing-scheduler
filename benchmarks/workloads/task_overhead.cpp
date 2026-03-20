#include <iostream>
#include <cassert>
#include <chrono>

#include "../include/scheduler/thread_pool.hpp"
#include "../include/scheduler/global_queue_thread_pool.hpp"

/*
	A benchmark to measure task management overhead within the scheduler.
	This benchmark submits many tiny arithmetic tasks and compares the
	scheduler's performance against a serial run and global queue scheduler.
*/

namespace benchmarks {

	void run_serial_task_overhead() {
		int counter{ 0 };
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for (std::size_t i = 0; i < 100000; i++)
			counter++;

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Serial scheduler concluded, time (sec) =  " << time << std::endl;
	}

	void run_global_queue_task_overhead(int thread_count) {
		scheduler::GlobalThreadPool globalPool(4);
		std::atomic<int> counter{ 0 };

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for (std::size_t i = 0; i < 100000; i++)
			globalPool.submit([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });
		globalPool.wait_for_all();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		assert(counter.load(std::memory_order_relaxed) == 100000);

		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Global queue scheduler concluded, time (sec) =  " << time << std::endl;
	}

	void run_work_stealing_task_overhead(int thread_count) {
		scheduler::ThreadPool pool(thread_count);
		std::atomic<int> counter{ 0 };

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for (std::size_t i = 0; i < 100000; i++)
			pool.submit([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });

		pool.wait_for_all();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		assert(counter.load(std::memory_order_relaxed) == 100000);

		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Work Stealing Scheduler concluded, time (sec) =  " << time << std::endl;
	}

	void run_task_overhead() {
		std::cout << "Running Task Overhead benchmark...\n";

		// Work Stealing
		run_work_stealing_task_overhead(4);

		// Work Sharing
		run_global_queue_task_overhead(4);

		// Serial
		run_serial_task_overhead();
	}

} // namespace benchmarks