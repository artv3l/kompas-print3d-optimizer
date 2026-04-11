#pragma once

#include <functional>

class ActionLock final {
public:
	ActionLock();
	ActionLock(std::function<void()>&& func);
	ActionLock(const ActionLock& other) = delete;
	ActionLock(ActionLock&& other) noexcept;

	~ActionLock();

	ActionLock& operator=(const ActionLock& other) = delete;
	ActionLock& operator=(ActionLock&& other) noexcept = delete;

private:
	std::function<void()> m_function;
};
