#include <iostream>

#include "workloads/task_overhead.hpp"
#include "workloads/monte_carlo_pi.hpp"

using namespace std;

int main() {
    benchmarks::run_monte_carlo_pi();

    //benchmarks::run_task_overhead();
    return 0;
}