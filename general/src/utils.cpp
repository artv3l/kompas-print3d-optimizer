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

std::pair<std::string, std::string> splitFileNameAndRemoveExtension(std::string fileName) {
	size_t lastSlashIndex = fileName.find_last_of('\\');
	size_t lastDotIndex = fileName.find_last_of('.');
	return std::make_pair(fileName.substr(0, lastSlashIndex), fileName.substr(lastSlashIndex + 1, lastDotIndex - lastSlashIndex - 1));
}
