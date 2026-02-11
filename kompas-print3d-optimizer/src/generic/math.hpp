#pragma once

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

#include "utils.hpp"

template <typename T>
T::value_type calcTriangleArea(const T& a, const T& b, const T& c)
{
	auto ab = b - a;
	auto ac = c - a;
	return glm::length(glm::cross(ab, ac)) / static_cast<T::value_type>(2.0);
}

// Угол между векторами в радианах
template <typename T>
T::value_type calcAngleBetween(const T& a, const T& b)
{
	auto dot = glm::dot(a, b);
	auto len = glm::length(a) * glm::length(b);
	if (doubleEqual(len, 0.0))
		return 0.0;
	return std::acos(std::clamp(dot / len, T::value_type(-1), T::value_type(1)));
}
