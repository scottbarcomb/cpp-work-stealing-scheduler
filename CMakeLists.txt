# CMakeList.txt : CMake project for cpp-work-stealing-scheduler, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cpp-work-stealing-scheduler")

# Add source to this project's executable.
add_executable (scheduler_tests "tests/basic_submit.cpp")
add_executable (scheduler_benchmarks "benchmarks/benchmark_main.cpp" "benchmarks/workloads/task_overhead.cpp")

# TODO: Add tests and install targets if needed.

# Add libraries
add_library(scheduler_lib "src/work_stealing_deque.cpp" "src/thread_pool.cpp" "src/global_queue_thread_pool.cpp")

# Install targets
target_include_directories(scheduler_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(scheduler_lib PUBLIC cxx_std_20)
target_link_libraries(scheduler_tests PRIVATE scheduler_lib)

target_include_directories(scheduler_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
target_link_libraries(scheduler_benchmarks PRIVATE scheduler_lib)
