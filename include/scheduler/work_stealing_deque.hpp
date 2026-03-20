#pragma once
#include <deque>
#include <functional>
#include <mutex>
#include <optional>

namespace scheduler {

	using Task = std::function<void()>;

	class WorkStealingDeque {
	public:
		WorkStealingDeque() = default;

		void push(Task task);
		std::optional<Task> pop();
		std::optional<Task> steal();

		bool empty() const;
		std::size_t size() const;

	private:
		mutable std::mutex mutex_;
		std::deque<Task> deque_;
	};

} // namespace scheduler