#include "perfomance.hpp"

#include <chrono>

namespace perfomance
{
ActionLock measureTime(MeasureTimeFunc func)
{
	using namespace std::chrono;

	auto start = high_resolution_clock::now();
	return ActionLock([start, func]() {
		auto stop = high_resolution_clock::now();
		func(stop - start);
	});
}
}
