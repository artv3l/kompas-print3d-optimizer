#ifndef LIN_ALG_HPP
#define LIN_ALG_HPP

struct Vec2d {
    double x, y;
    double w;

    Vec2d(double x_, double y_);
    Vec2d(double x_, double y_, double w_);
};

struct TransformationMatrix2d {
    double matrix[3][3];

    TransformationMatrix2d(double angle, double x, double y);
    
    Vec2d operator*(const Vec2d vec2d) const;
};

#endif /* LIN_ALG_HPP */
