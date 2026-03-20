#pragma once

namespace benchmarks {

	void run_monte_carlo_pi();
	void run_serial_monte_carlo();
	void run_global_queue_monte_carlo(int thread_count);
	void run_work_stealing_monte_carlo(int thread_count);

} // namespace benchmarks