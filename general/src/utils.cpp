#include "utils.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

bool doubleEqual(double a, double b, double epsilon) {
	return (abs(a - b) < epsilon);
}

double degreeToRadian(double degree) {
	return degree * M_PI / 180.0;
}
