#include "utils.hpp"

#include <cmath>

bool doubleEqual(double a, double b, double epsilon) {
	return (abs(a - b) < epsilon);
}
