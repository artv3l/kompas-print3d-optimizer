#include "LinAlg.hpp"

#include <cmath>

Vec2d::Vec2d(double x_, double y_) :
    x(x_), y(y_), w(1.0)
{}

Vec2d::Vec2d(double x_, double y_, double w_):
    x(x_), y(y_), w(w_) 
{}

TransformationMatrix2d::TransformationMatrix2d(double angle, double x, double y) {
    matrix[0][0] = std::cos(angle); matrix[0][1] = -std::sin(angle); matrix[0][2] = x;
    matrix[1][0] = std::sin(angle); matrix[1][1] = std::cos(angle); matrix[1][2] = y;
    matrix[2][0] = 0.0; matrix[2][1] = 0.0; matrix[2][2] = 1.0;
}

Vec2d TransformationMatrix2d::operator*(const Vec2d vec2d) const {
    double x = (matrix[0][0] * vec2d.x) + (matrix[0][1] * vec2d.y) + (matrix[0][2] * vec2d.w);
    double y = (matrix[1][0] * vec2d.x) + (matrix[1][1] * vec2d.y) + (matrix[1][2] * vec2d.w);
    double w = (matrix[2][0] * vec2d.x) + (matrix[2][1] * vec2d.y) + (matrix[2][2] * vec2d.w);
    return Vec2d(x, y, w);
}
