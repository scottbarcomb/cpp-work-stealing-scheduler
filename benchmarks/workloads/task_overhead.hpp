#pragma once

namespace benchmarks {

	void run_task_overhead();
	void run_serial_task_overhead();
	void run_global_queue_task_overhead(int thread_count);
	void run_work_stealing_task_overhead(int thread_count);

} // namespace benchmarks