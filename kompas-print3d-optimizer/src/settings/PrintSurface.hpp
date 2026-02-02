#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>

#include <glm/vec3.hpp>

struct PlaneEq {
    double a, b, c, d;

    PlaneEq(kapi::ksFaceDefinitionPtr face);

    bool operator==(const PlaneEq& other) const;
    bool operator!=(const PlaneEq& other) const;

    void invert();
};

struct PrintSurface {
    kapi::ksFaceDefinitionPtr face;
    PlaneEq eq;
};

/*
  Статистика выбранной ориентации детали (плоскости печати).
  Нужно учитывать, что деталь может измениться. Тогда эта статистика становится неактуальной.
*/
struct OrientationStat {
    double bodyArea = 0.0; // Общая площадь всей детали
    double supportArea = 0.0; // Площадь поддержек
    // ? Максимальный угол нависаний
};

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);
OrientationStat calcOrientationStat(kapi::ksBodyPtr body, const glm::vec3 & direction);

#endif /* PRINT_SURFACE_HPP */
