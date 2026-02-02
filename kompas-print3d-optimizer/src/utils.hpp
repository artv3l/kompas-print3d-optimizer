#ifndef UTILS_HPP
#define UTILS_HPP

#include <utility>
#include <string>
#include <stdexcept>
#include <cassert>

class NullptrException : public std::runtime_error {
	using runtime_error::runtime_error;
};

template <typename Source>
void checkPtr(Source ptr) {
	if (!ptr) {
		assert(false);
		throw NullptrException("Unexpected nullptr");
	}
}

template <typename Result, typename Source>
Result checkCast(Source ptr) {
	checkPtr(ptr);
	Result result = ptr;
	checkPtr(result);
	return result;
}

bool doubleEqual(double a, double b, double epsilon = 0.00001);
double degreeToRadian(double degree);
std::pair<std::string, std::string> splitFileNameAndRemoveExtension(std::string fileName);

#endif /* UTILS_HPP */
