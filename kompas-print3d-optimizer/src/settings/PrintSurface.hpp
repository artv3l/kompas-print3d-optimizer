#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>
#include <span>
#include <memory>

#include <KsAPI.h>

struct PlaneEq {
	double a, b, c, d;

	PlaneEq(kapi::ksFaceDefinitionPtr face);
	PlaneEq(ksapi::IFacePtr face);

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
