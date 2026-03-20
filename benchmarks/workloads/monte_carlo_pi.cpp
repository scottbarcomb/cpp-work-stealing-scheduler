#include "../include/scheduler/thread_pool.hpp"
#include "../include/scheduler/global_queue_thread_pool.hpp"

#include <random>
#include <iostream>
#include <chrono>

constexpr auto INTERVAL = 10000;

namespace benchmarks {

	void run_serial_monte_carlo() {
		double rand_x, rand_y, origin_dist, pi;
		int circle_points = 0, square_points = 0;

		// Initialize random number generator
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr(-1, 1);

		// Run the simulation
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for (size_t i = 0; i < (INTERVAL * INTERVAL); i++) {
			// Randomly gen x and y vals
			rand_x = distr(gen);
			rand_y = distr(gen);

			// Distance between origin and (x, y)
			origin_dist = rand_x * rand_x + rand_y * rand_y;

			// Check if point lies within circle
			if (origin_dist <= 1)
				circle_points++;
			square_points++;
		}

		// Estimate pi
		pi = double(4 * circle_points) / square_points;

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Serial simulation time (sec): " << time << std::endl;
		std::cout << "Estimate of pi: " << pi << std::endl;
	}

	void run_global_queue_monte_carlo(int thread_count) {
		std::atomic<int> circle_points{ 0 };
		std::atomic<int> square_points{ 0 };

		// Initialize random number generator
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr(-1, 1);

		// Initialize the thread pool
		scheduler::GlobalThreadPool pool(thread_count);
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for (size_t i = 0; i < INTERVAL; i++) {
			pool.submit([&circle_points, &square_points, &distr, &gen] {
				for (size_t i = 0; i < INTERVAL; i++) {
					double rand_x = distr(gen);
					double rand_y = distr(gen);
					double origin_dist = rand_x * rand_x + rand_y * rand_y;

					if (origin_dist <= 1)
						circle_points.fetch_add(1, std::memory_order_relaxed);
					square_points.fetch_add(1, std::memory_order_relaxed);
				}
			});
		}
		pool.wait_for_all();

		double pi = double(4 * circle_points.load(std::memory_order_relaxed)) / square_points.load(std::memory_order_relaxed);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Global queue simulation time (sec): " << time << std::endl;
		std::cout << "Estimate of pi: " << pi << std::endl;
	}

	void run_work_stealing_monte_carlo(int thread_count) {
		std::atomic<int> circle_points{ 0 };
		std::atomic<int> square_points{ 0 };

		// Initialize random number generator
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr(-1, 1);

		// Initialize the thread pool
		scheduler::ThreadPool pool(thread_count);
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for (size_t i = 0; i < INTERVAL; i++) {
			pool.submit([&circle_points, &square_points, &distr, &gen] {
				for (size_t i = 0; i < INTERVAL; i++) {
					double rand_x = distr(gen);
					double rand_y = distr(gen);
					double origin_dist = rand_x * rand_x + rand_y * rand_y;

					if (origin_dist <= 1)
						circle_points.fetch_add(1, std::memory_order_relaxed);
					square_points.fetch_add(1, std::memory_order_relaxed);
				}
			});
		}
		pool.wait_for_all();

		double pi = double(4 * circle_points.load(std::memory_order_relaxed)) / square_points.load(std::memory_order_relaxed);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		double time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		std::cout << "Work Stealing simulation time (sec): " << time << std::endl;
		std::cout << "Estimate of pi: " << pi << std::endl;
	}
	
	void run_monte_carlo_pi() {
		run_serial_monte_carlo();
		std::cout << "\n";
		run_global_queue_monte_carlo(4);
		std::cout << "\n";
		run_work_stealing_monte_carlo(4);
	}

} // namespace benchmarks