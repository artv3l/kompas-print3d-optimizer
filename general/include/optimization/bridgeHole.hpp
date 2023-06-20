#ifndef BRIDGE_HOLE_HPP
#define BRIDGE_HOLE_HPP

#include <list>

#include "apiutil/Sketch.hpp"
#include "SettingsManager.hpp"

struct BridgeHoleFillTarget {
    ksLoopPtr loop;
    ksFaceDefinitionPtr face;
};

struct BridgeHoleBuildTarget {
    ksLoopPtr innerLoop;
    ksLoopPtr outerLoop;
    ksFaceDefinitionPtr face;
};

enum class HoleType {
    CIRCLE,
    NOT_CIRCLE,
    ALL,
};

bool loopIsCircle(ksLoopPtr loop);
bool checkFaceWithHole(ksFaceDefinitionPtr face, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool isHoleDirect(ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool checkHoleLoop(ksDocument3DPtr document3d, ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);

ksEntityPtr cutExtrusion(ksPartPtr part, ksEntityPtr sketchEntity, bool normalDirection, double depth);

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, HoleType holeType);
void fillBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleTargets, double extrusionDepth);
void optimizeBridgeHoleFill(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, const Settings& settings, HoleType holeType);

bool isOuterLoopForBuild(ksLoopPtr loop);
void drawLoopProjection(ksSketchDefinitionPtr sketchDef, ksLoopPtr loop);
std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace);
ICirclePtr drawThinInnerCircleProjection(Sketch sketch, BridgeHoleBuildTarget target);
void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void closeContour(ILineSegmentsPtr lineSegments, std::list<std::pair<double, ILineSegmentPtr>> points, double y, long partnerIndex);
void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void bridgeHoleBuildDrawSketch2(KompasObjectPtr kompas, Sketch sketch, BridgeHoleBuildTarget target, int angleCount);
void buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleTargets, double stepDepth);
void optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, const Settings& settings);

#endif /* BRIDGE_HOLE_HPP */
