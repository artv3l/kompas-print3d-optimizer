#include "utils.hpp"

#include <utility>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

bool doubleEqual(double a, double b, double epsilon) {
	return (abs(a - b) < epsilon);
}

double degreeToRadian(double degree) {
	return degree * M_PI / 180.0;
}
