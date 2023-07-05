#ifndef UTILS_HPP
#define UTILS_HPP

#include <utility>
#include <string>

bool doubleEqual(double a, double b, double epsilon = 0.00001);
double degreeToRadian(double degree);
std::pair<std::string, std::string> splitFileNameAndRemoveExtension(std::string fileName);

#endif /* UTILS_HPP */
