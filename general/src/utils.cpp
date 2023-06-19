#include "stdafx.h"
#include "utils.hpp"

bool doubleEqual(double a, double b, double epsilon) {
	return (abs(a - b) < epsilon);
}
