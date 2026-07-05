#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

#include <eigen3/Eigen/Dense>

#include "geometry3d.hpp"

namespace math
{
bool equal(double a, double b, double epsilon = 0.00001);
double toRadians(double angleInDegrees);
// Получить острый угол от 0 до pi/2
double toAcuteAngle(double angleInRadians);
// Привести значение baseValue из промежутка [baseBegin, baseBegin + baseLength] в промежуток [resultBegin, resultBegin + resultLength]
double convertRanges(double baseValue, double baseBegin, double baseLength, double resultBegin, double resultLength);
double polygonArea(std::span<Eigen::Vector2d> points);
}
