#include "ActionLock.hpp"

ActionLock::ActionLock():
	m_function()
{
}

ActionLock::ActionLock(std::function<void()>&& func):
	m_function(std::move(func))
{
}

ActionLock::ActionLock(ActionLock&& other) noexcept:
	m_function(std::move(other.m_function))
{
}

ActionLock::~ActionLock()
{
	if (m_function)
		m_function();
}
