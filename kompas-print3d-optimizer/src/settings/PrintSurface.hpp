#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>
#include <span>
#include <memory>

#include <glm/vec3.hpp>

#include "oglwrap/Mesh.hpp"
#include "generic/math.hpp"
#include "generic/enums.hpp"


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

std::pair<int, int> countPointsOnEachSide(kapi::ksPartPtr part, const PlaneEq& planeEq);
PrintSurface getSelectedPrintSurface(kapi::ksDocument3DPtr document3d);

#endif /* PRINT_SURFACE_HPP */
