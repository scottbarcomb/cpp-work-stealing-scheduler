#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <array>
#include <vector>

#include "../include/scheduler/thread_pool.hpp"
#include "../include/scheduler/global_queue_thread_pool.hpp"

using namespace std;

void test_basic_functionality() {
	scheduler::ThreadPool pool(4);
	std::atomic<int> counter{ 0 };

	for (std::size_t i = 0; i < 100; i++)
		pool.submit([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });

	pool.wait_for_all();

	assert(counter.load(std::memory_order_relaxed) == 100);
}

void test_no_lost_tasks() {
	scheduler::ThreadPool pool(8);
	std::atomic<int> counter{ 0 };

	for (std::size_t i = 0; i < 100000; i++)
		pool.submit([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });

	pool.wait_for_all();

	assert(counter.load(std::memory_order_relaxed) == 100000);
}

void test_work_stealing() {
	scheduler::ThreadPool pool(4);
	std::atomic<int> counter{ 0 };

	for (std::size_t i = 0; i < 1000; i++)
		pool.forceSubmitOneThread([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });

	pool.wait_for_all();

	assert(counter.load(std::memory_order_relaxed) == 1000);
}

void test_recursive_task_spawn() {
	scheduler::ThreadPool pool(4);
	std::array<std::atomic<int>, 100> seen{};
	for (auto& x : seen)
		x.store(0, std::memory_order_relaxed);

	for (int i = 0; i < 50; i++) {
		pool.submit([&pool, &seen, i] {
			seen.at(i).fetch_add(1, std::memory_order_relaxed);
			pool.submit([&seen, i] {
				seen.at(50 + i).fetch_add(1, std::memory_order_relaxed);
			});
		});
	}

	pool.wait_for_all();

	for (const auto& x : seen)
		assert(x.load(std::memory_order_relaxed) == 1);

	auto stats = pool.snapshot_stats();
	for (std::size_t i = 0; i < stats.size(); i++)
		std::cout << "Thread " << i << " idled " << stats.at(i).idle_loops << " times." << std::endl;
	std::cout << "\n";
}

void test_durability() {
	for (int run = 0; run < 10; run++)
		test_recursive_task_spawn();
}

void test_global_queue() {
	scheduler::GlobalThreadPool pool(4);
	std::atomic<int> counter{ 0 };

	for (std::size_t i = 0; i < 100; i++)
		pool.submit([&counter] {counter.fetch_add(1, std::memory_order_relaxed); });

	pool.wait_for_all();

	assert(counter.load(std::memory_order_relaxed) == 100);

	auto stats = pool.snapshot_stats();
	for (std::size_t i = 0; i < stats.size(); i++)
		std::cout << "Thread " << i << " stole " << stats.at(i).successful_steals << " tasks." << std::endl;
	std::cout << "\n";
}

int main() {
	// test_basic_functionality();
	// test_no_lost_tasks();
	// test_work_stealing();
	// test_recursive_task_spawn();
	// test_durability();
	test_global_queue();
	return 0;
}