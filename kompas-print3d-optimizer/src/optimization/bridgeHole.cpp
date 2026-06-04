#include "bridgeHole.hpp"

#include <list>
#include <sstream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include "kapiwrap/Macro.hpp"
#include "kapiwrap/ConstraintsCreator.hpp"
#include "kapiwrap/Sketch.hpp"

#include "concaveAngle.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"

const char* MACRO_NAME_BRIDGE_HOLE_FILL = "Закрытие нависающих отвертий диафрагмой";
const char* MACRO_NAME_BRIDGE_HOLE_FILL_ELEMENT = "Отверстие";
const char* MACRO_NAME_BRIDGE_HOLE_BUILD = "Достройка нависающих отверстий";
const char* MACRO_NAME_BRIDGE_HOLE_BUILD_ELEMENT = "Отверстие";

bool loopIsCircle(kapi::ksLoopPtr loop) {
	kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();
	if (edgesCount != 1) {
		return false;
	}
	kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(0));
	if (!edge->IsCircle()) {
		return false;
	}
	return true;
}

bool checkFaceWithHole(kapi::ksFaceDefinitionPtr face, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer) {
	if (!face->IsPlanar() || (face == printFace)) {
		return false;
	}
	measurer->SetObject1(printFace);
	measurer->SetObject2(face);
	measurer->Calc();
	double angle = measurer->angle;
	if (!(math::equal(angle, 0.0) || math::equal(angle, 180))) {
		return false;
	}
	return true;
}

bool isHoleDirect(kapi::ksFaceDefinitionPtr face, kapi::ksLoopPtr loop, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer) {
	kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();
	for (int holeEdgeIndex = 0; holeEdgeIndex < edgesCount; holeEdgeIndex++) {
		kapi::ksEdgeDefinitionPtr holeEdge(edges->GetByIndex(holeEdgeIndex));

		kapi::ksFaceDefinitionPtr holeFace(holeEdge->GetAdjacentFace(false));
		if (holeFace == face) {
			holeFace = holeEdge->GetAdjacentFace(true);
		}

		if (holeFace->IsCylinder()) {
			continue;
		}
		if (!holeFace->IsPlanar()) {
			return false;
		}

		measurer->SetObject1(printFace);
		measurer->SetObject2(holeFace);
		measurer->Calc();
		double angle = measurer->angle;
		if (!(math::equal(angle, 90.0) || math::equal(angle, 270.0))) {
			return false;
		}
	}
	return true;
}

bool checkHoleLoop(kapi::ksDocument3DPtr document3d, kapi::ksFaceDefinitionPtr face, kapi::ksLoopPtr loop, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer) {
	if (!isHoleDirect(face, loop, printFace, measurer)) {
		return false;
	}

	kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();

	for (int holeEdgeIndex = 0; holeEdgeIndex < edgesCount; holeEdgeIndex++) {
		kapi::ksEdgeDefinitionPtr holeEdge(edges->GetByIndex(holeEdgeIndex));

		kapi::ksFaceDefinitionPtr holeFace(holeEdge->GetAdjacentFace(false));
		if (holeFace == face) {
			holeFace = holeEdge->GetAdjacentFace(true);
		}

		if (isConcaveAngle(document3d, holeEdge)) {
			return false;
		}

		measurer->SetObject1(printFace);
		measurer->SetObject2(holeEdge);
		measurer->Calc();
		double distanceToHoleEdge = measurer->distance;

		kapi::ksEdgeCollectionPtr edges2(holeFace->EdgeCollection());
		for (int edge2Index = 0; edge2Index < edges2->GetCount(); edge2Index++) {
			kapi::ksEdgeDefinitionPtr edge2(edges2->GetByIndex(edge2Index));
			if (edge2 == holeEdge) {
				continue;
			}

			measurer->SetObject1(printFace);
			measurer->SetObject2(edge2);
			measurer->Calc();
			if (edge2->IsStraight()) {
				double angle = measurer->angle;
				if (math::equal(angle, 90.0) || math::equal(angle, 270.0)) {
					continue;
				}
			}
			double distanceToEdge2 = measurer->distance;
			if (distanceToEdge2 < distanceToHoleEdge) {
				return false;
			}
		}
	}
	return true;
}

kapi::ksEntityPtr cutExtrusion(kapi::ksPartPtr part, kapi::ksEntityPtr sketchEntity, bool normalDirection, Settings& settings, int multiplier) {
	kapi::ksEntityPtr extrusionEntity(part->NewEntity(kapi::o3d_cutExtrusion));
	kapi::ksCutExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
	extrusionDef->cut = true;
	extrusionDef->chooseType = kapi::ksChBodiesAndParts;
	if (normalDirection) {
		extrusionDef->directionType = kapi::dtNormal;
	} else {
		extrusionDef->directionType = kapi::dtReverse;
	}
	double depth = settings.getDoubleSetting(si::bridgeHoleBuildLayersCount.name)->getValue() *
		settings.getDoubleSetting(si::layerHeight.name)->getValue() * multiplier;
	extrusionDef->SetSideParam(normalDirection, kapi::etBlind, depth, 0, false);
	extrusionDef->SetSketch(sketchEntity);
	extrusionEntity->Create();

	{ // Привязываем размеры к переменным
		kapi::ksFeaturePtr feature(extrusionEntity->GetFeature());
		kapi::ksVariableCollectionPtr variableCollection(feature->VariableCollection);
		kapi::ksVariablePtr variable(variableCollection->GetByIndex(1)); // Индекс=3 - "Расстояние 1"

		std::ostringstream oss;
		oss << multiplier << " * ("
			<< settings.getDoubleSetting(si::bridgeHoleBuildLayersCount.name)->getExpression() << " * "
			<< settings.getDoubleSetting(si::layerHeight.name)->getExpression()
			<< ")";
		variable->Expression = oss.str().c_str();
	}

	return extrusionEntity;
}

/* Закрытие нависающих отверстий тонким слоем материала */

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, HoleType holeType) {
	kapi::ksMeasurerPtr measurer(part->GetMeasurer());

	kapi::ksBodyPtr body = part->GetMainBody();
	kapi::ksFaceCollectionPtr faces = body->FaceCollection();
	int facesCount = faces->GetCount();

	std::list<BridgeHoleFillTarget> bridgeHoleFillTargets;

	for (int faceIndex = 0; faceIndex < facesCount; faceIndex++) {
		kapi::ksFaceDefinitionPtr face = faces->GetByIndex(faceIndex);
		if (!checkFaceWithHole(face, printFace, measurer)) {
			continue;
		}

		kapi::ksLoopCollectionPtr loops(face->LoopCollection());
		for (int loopIndex = 0; loopIndex < loops->GetCount(); loopIndex++) {
			kapi::ksLoopPtr innerLoop(loops->GetByIndex(loopIndex));
			if (innerLoop->IsOuter()) {
				continue;
			}

			if (holeType == HoleType::CIRCLE) {
				if (!loopIsCircle(innerLoop)) {
					continue;
				}
			} else if (holeType == HoleType::NOT_CIRCLE) {
				if (loopIsCircle(innerLoop)) {
					continue;
				}
			}

			if (checkHoleLoop(document3d, face, innerLoop, printFace, measurer)) {
				bridgeHoleFillTargets.push_back(BridgeHoleFillTarget{innerLoop, face});
			}
		}
	}
	return bridgeHoleFillTargets;
}

kapi::ksEntityPtr fillBridgeHoles(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleFillTargets, Settings& settings) {
	Macro macro(part, MACRO_NAME_BRIDGE_HOLE_FILL, true);

	for (BridgeHoleFillTarget target : bridgeHoleFillTargets) {
		Macro macroElement(part, MACRO_NAME_BRIDGE_HOLE_FILL_ELEMENT, true);

		Sketch sketch(kompas, part, target.face);
		macroElement.add(sketch.entity);

		kapi::ksEdgeCollectionPtr edges(target.loop->EdgeCollection());
		int edgesCount = edges->GetCount();
		for (int edgeIndex = 0; edgeIndex < edgesCount; edgeIndex++) {
			sketch.definition->AddProjectionOf(edges->GetByIndex(edgeIndex));
		}
		sketch.endEdit();

		double extrusionDepth = settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getValue() * settings.getDoubleSetting(si::layerHeight.name)->getValue();
		kapi::ksEntityPtr extrusionEntity(part->NewEntity(kapi::o3d_bossExtrusion));
		kapi::ksBossExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
		extrusionDef->chooseType = kapi::ksChBodiesAndParts;
		extrusionDef->directionType = kapi::dtReverse;
		extrusionDef->SetSideParam(false, kapi::etBlind, extrusionDepth, 0, false);
		extrusionDef->SetSketch(sketch.entity);
		extrusionEntity->Create();
		{ // Привязываем размеры к переменным
			kapi::ksFeaturePtr feature(extrusionEntity->GetFeature());
			kapi::ksVariableCollectionPtr variableCollection(feature->VariableCollection);
			kapi::ksVariablePtr variable(variableCollection->GetByIndex(3)); // Индекс=3 - "Расстояние 2"

			std::ostringstream oss;
			oss << settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getExpression() << " * " << settings.getDoubleSetting(si::layerHeight.name)->getExpression();
			variable->Expression = oss.str().c_str();
		}
		macroElement.add(extrusionEntity);

		macro.add(macroElement);
	}
	return macro.getEntity();
}

kapi::ksEntityPtr optimizeBridgeHoleFill(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, Settings& settings, HoleType holeType) {
	std::list<BridgeHoleFillTarget> targets = getBridgeHoleFillTargets(document3d, part, settings.getPrintSurface()->face, holeType);
	if (targets.empty()) {
		return nullptr;
	}
	return fillBridgeHoles(kompas, part, targets, settings);
}

/* Достройка нависающих отверстий для печати мостами */

bool isOuterLoopForBuild(kapi::ksLoopPtr loop) {
	if (loopIsCircle(loop)) {
		return true;
	}
	kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
	for (int edgeIndex = 0; edgeIndex < edges->GetCount(); edgeIndex++) {
		kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(edgeIndex));
		if (!edge->IsStraight() && !edge->IsArc()) {
			return false;
		}
	}
	return true;
}

std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace) {
	kapi::ksMeasurerPtr measurer(part->GetMeasurer());

	kapi::ksBodyPtr body = part->GetMainBody();
	kapi::ksFaceCollectionPtr faces = body->FaceCollection();
	int facesCount = faces->GetCount();

	std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets;

	for (int faceIndex = 0; faceIndex < facesCount; faceIndex++) {
		kapi::ksFaceDefinitionPtr face = faces->GetByIndex(faceIndex);
		if (!checkFaceWithHole(face, printFace, measurer)) {
			continue;
		}

		kapi::ksLoopCollectionPtr loops(face->LoopCollection());
		int loopsCount = loops->GetCount();

		if (loopsCount != 2) {
			continue;
		}

		kapi::ksLoopPtr testLoop(loops->GetByIndex(0));
		kapi::ksLoopPtr innerLoop, outerLoop;
		if (testLoop->IsOuter()) {
			outerLoop = testLoop;
			innerLoop = loops->GetByIndex(1);
		} else {
			innerLoop = testLoop;
			outerLoop = loops->GetByIndex(1);
		}

		if (!loopIsCircle(innerLoop) || !isOuterLoopForBuild(outerLoop)) {
			continue;
		}

		if (loopIsCircle(outerLoop)) {
			kapi::ksEdgeCollectionPtr innerEdges(innerLoop->EdgeCollection());
			kapi::ksEdgeDefinitionPtr innerEdge(innerEdges->GetByIndex(0));
			double innerRadius = innerEdge->GetLength(kapi::ksLengthUnitsEnum::ksLUnMM) / (2 * M_PI);

			kapi::ksEdgeCollectionPtr outerEdges(outerLoop->EdgeCollection());
			kapi::ksEdgeDefinitionPtr outerEdge(outerEdges->GetByIndex(0));
			double outerRadius = outerEdge->GetLength(kapi::ksLengthUnitsEnum::ksLUnMM) / (2 * M_PI);

			if ((2 * innerRadius * innerRadius) >= (outerRadius * outerRadius)) {
				continue;
			}
		}

		if (checkHoleLoop(document3d, face, innerLoop, printFace, measurer)) {
			bridgeHoleBuildTargets.push_back(BridgeHoleBuildTarget{innerLoop, outerLoop, face});
		}
	}
	return bridgeHoleBuildTargets;
}

void drawLoopProjection(kapi::ksSketchDefinitionPtr sketchDef, kapi::ksLoopPtr loop) {
	kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
	for (int i = 0; i < edges->GetCount(); i++) {
		kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
		sketchDef->AddProjectionOf(edge);
	}
}

kapi::ICirclePtr drawThinInnerCircleProjection(Sketch sketch, BridgeHoleBuildTarget target) {
	kapi::ksEdgeCollectionPtr innerEdges(target.innerLoop->EdgeCollection());
	kapi::ksEdgeDefinitionPtr innerEdge(innerEdges->GetByIndex(0));
	sketch.definition->AddProjectionOf(innerEdge);

	kapi::ICirclesPtr circles(sketch.drawingContainer->Circles);
	kapi::ICirclePtr innerCircle(circles->GetCircle(0));
	innerCircle->Style = kapi::ksCurveStyleEnum::ksCSThin;
	innerCircle->Update();

	return innerCircle;
}

void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, kapi::ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
	drawLoopProjection(sketch.definition, target.outerLoop);

	kapi::ICirclesPtr circles(sketch.drawingContainer->Circles);
	kapi::ICirclePtr outerCircle;
	for (int circleIndex = 0; circleIndex < circles->Count; circleIndex++) {
		kapi::ICirclePtr circle(circles->GetCircle(circleIndex));
		if (circle != innerCircle) {
			outerCircle = circle;
		}
	}
	outerCircle->Style = kapi::ksCurveStyleEnum::ksCSThin;
	outerCircle->Update();

	kapi::ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);

	kapi::ILineSegmentPtr lineSegment1(lineSegments->Add());
	lineSegment1->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment1->Y1 = outerCircle->Yc - innerCircle->Radius;
	lineSegment1->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment1->Y2 = outerCircle->Yc - innerCircle->Radius;
	lineSegment1->Update();
	ConstraintsCreator constrCreator(lineSegment1);
	constrCreator.pointOnCurve(0, outerCircle);
	constrCreator.pointOnCurve(1, outerCircle);
	constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);

	kapi::ILineSegmentPtr lineSegment2(lineSegments->Add());
	lineSegment2->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment2->Y1 = outerCircle->Yc + innerCircle->Radius;
	lineSegment2->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment2->Y2 = outerCircle->Yc + innerCircle->Radius;
	lineSegment2->Update();
	constrCreator = ConstraintsCreator(lineSegment2);
	constrCreator.pointOnCurve(0, outerCircle);
	constrCreator.pointOnCurve(1, outerCircle);
	constrCreator.parallel(lineSegment1);
	constrCreator.tangentTwoCurves(innerCircle);

	kapi::IArcsPtr arcs(sketch.drawingContainer->Arcs);
	{
		kapi::IArcPtr arc(arcs->Add());
		arc->Xc = outerCircle->Xc; arc->Yc = outerCircle->Yc;
		arc->Radius = outerCircle->Radius;
		arc->X1 = lineSegment1->X1; arc->Y1 = lineSegment1->Y1;
		arc->X2 = lineSegment2->X1; arc->Y2 = lineSegment2->Y1;
		arc->Direction = false;
		arc->Update();
		constrCreator = ConstraintsCreator(arc);
		constrCreator.mergePoints(0, outerCircle, 0);
		constrCreator.mergePoints(1, lineSegment1, 0);
		constrCreator.mergePoints(2, lineSegment2, 0);
	}
	{
		kapi::IArcPtr arc(arcs->Add());
		arc->Xc = outerCircle->Xc; arc->Yc = outerCircle->Yc;
		arc->Radius = outerCircle->Radius;
		arc->X1 = lineSegment1->X2; arc->Y1 = lineSegment1->Y2;
		arc->X2 = lineSegment2->X2; arc->Y2 = lineSegment2->Y2;
		arc->Direction = true;
		arc->Update();
		constrCreator = ConstraintsCreator(arc);
		constrCreator.mergePoints(0, outerCircle, 0);
		constrCreator.mergePoints(1, lineSegment1, 1);
		constrCreator.mergePoints(2, lineSegment2, 1);
	}
}

void closeContour(kapi::ILineSegmentsPtr lineSegments, std::list<MergePointInfo> points) {
	size_t nPoints = points.size();
	if ((nPoints < 2) || (nPoints % 2 != 0)) {
		return;
	}
	points.sort([](const MergePointInfo& lhs, const MergePointInfo& rhs) {
		if (math::equal(lhs.y, rhs.y)) {
			return lhs.x < rhs.x;
		}
		return lhs.y < rhs.y;
	});

	// Размеры всегда будут четными
	for (std::list<MergePointInfo>::const_iterator it = points.cbegin(); it != points.cend(); it++) {
		kapi::ILineSegmentPtr lineSegment(lineSegments->Add());
		lineSegment->X1 = it->x; lineSegment->Y1 = it->y;
		kapi::IDrawingObjectPtr drawingObject1 = it->drawingObject; int drawingObjectIndex1 = it->drawingObjectIndex;
		it++;
		lineSegment->X2 = it->x; lineSegment->Y2 = it->y;
		lineSegment->Update();
		ConstraintsCreator c(lineSegment);
		c.mergePoints(0, drawingObject1, drawingObjectIndex1);
		c.mergePoints(1, it->drawingObject, it->drawingObjectIndex);
	}
}

std::pair<kapi::ILinePtr, kapi::ILinePtr> drawBasicLines(Sketch sketch, kapi::ICirclePtr innerCircle, double angle) {
	double y1 = innerCircle->Yc - innerCircle->Radius;
	double y2 = innerCircle->Yc + innerCircle->Radius;

	kapi::ILinesPtr lines(sketch.drawingContainer->Lines);

	/*
	ILinePtr horizontalLine(lines->Add());
	horizontalLine->X1 = innerCircle->Xc + 1; horizontalLine->Y1 = y1;
	horizontalLine->X2 = innerCircle->Xc - 1; horizontalLine->Y2 = y1;
	horizontalLine->Update();
	ConstraintsCreator constrCreator(horizontalLine);
	constrCreator.horizontal();
	constrCreator = ConstraintsCreator(innerCircle);
	constrCreator.pointOnCurve(0, horizontalLine);

	ILinePtr baseLine(lines->Add());
	baseLine->X1 = innerCircle->Xc + 1; baseLine->Y1 = y1;
	baseLine->X2 = innerCircle->Xc - 1; baseLine->Y2 = y1;
	baseLine->Update();
	constrCreator = ConstraintsCreator(baseLine);
	constrCreator = ConstraintsCreator(innerCircle);
	constrCreator.pointOnCurve(0, baseLine);

	ISymbols2DContainerPtr symbols2dContainer(sketch.view);
	ILineDimensionsPtr lineDimensions(symbols2dContainer->LineDimensions);
	IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);
	IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
	angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
	angleDim->BaseObject1 = horizontalLine;
	angleDim->BaseObject2 = baseLine;
	angleDim->Radius = 0;
	angleDim->X3 = 0; angleDim->Y3 = 0;
	if (angle > 90.0) {
		angleDim->DimensionType = ksAngleDimTypeEnum::ksADMaxAngle;
	} else {
		angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
	}
	angleDim->Update();
	constrCreator = ConstraintsCreator(angleDim);
	constrCreator.fixedDim();
	std::ostringstream oss;
	oss << angle;
	constrCreator.dimWithVariable(oss.str().c_str());
	*/

	kapi::ILinePtr line1(lines->Add());
	line1->X1 = innerCircle->Xc + 1; line1->Y1 = y1;
	line1->X2 = innerCircle->Xc - 1; line1->Y2 = y1;
	line1->Update();
	ConstraintsCreator constrCreator(line1);
	constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);
	
	kapi::ILinePtr line2(lines->Add());
	line2->X1 = innerCircle->Xc + 1; line2->Y1 = y2;
	line2->X2 = innerCircle->Xc - 1; line2->Y2 = y2;
	line2->Update();
	constrCreator = ConstraintsCreator(line2);
	constrCreator.parallel(line1);
	constrCreator.tangentTwoCurves(innerCircle);

	return std::make_pair(line1, line2);
}

bool pointInsideInterval(kapi::ksMathematic2DPtr math2d, double x, double y, kapi::ILinePtr line1, kapi::ILinePtr line2) {
	double distance1 = math2d->ksDistancePntLineForPoint(x, y, line1->X1, line1->Y1, line1->X2, line1->Y2);
	double distance2 = math2d->ksDistancePntLineForPoint(x, y, line2->X1, line2->Y1, line2->X2, line2->Y2);
	double intervalLength = math2d->ksDistancePntLineForPoint(line1->X1, line1->Y1, line2->X1, line2->Y1, line2->X2, line2->Y2);
	return math::equal(distance1 + distance2, intervalLength);
}

void processLineSegment(kapi::KompasObjectPtr kompas, Sketch1NotCircleInfo info, kapi::ILineSegmentPtr lineSegment) {
	kapi::ksMathematic2DPtr math2d = kompas->GetMathematic2D();
	
	kapi::ksDynamicArrayPtr dynArr1(kompas->GetDynamicArray(2));
	kapi::ksDynamicArrayPtr dynArr2(kompas->GetDynamicArray(2));
	int res1 = math2d->ksIntersectCurvCurv(lineSegment->Reference, info.line1->Reference, dynArr1);
	int res2 = math2d->ksIntersectCurvCurv(lineSegment->Reference, info.line2->Reference, dynArr2);

	// Отрезок полностью вне промежутка
	if (!pointInsideInterval(math2d, lineSegment->X1, lineSegment->Y1, info.line1, info.line2) &&
		!pointInsideInterval(math2d, lineSegment->X2, lineSegment->Y2, info.line1, info.line2) &&
		(res1 == 0) && (res2 == 0))
	{
		lineSegment->Style = kapi::ksCurveStyleEnum::ksCSThin;
		lineSegment->Update();
		return;
	}

	// Отрезок полностью внутри промежутка
	if ((res1 != 1) && (res2 != 1)) {
		return;
	}

	lineSegment->Style = kapi::ksCurveStyleEnum::ksCSThin;
	lineSegment->Update();

	/*
	  Для всех отрезков(newLineSegment), которые построены на основе отрезков(lineSegment), пересекающих line1(y1) и line2(y2):
	  - Первая точка (index в ограничениях равен 0, координаты при создании: X1 и Y1) лежит на line1,
	  - Вторая точка лежит на line2.
	*/

	kapi::ILineSegmentsPtr lineSegments(info.sketch.drawingContainer->LineSegments);
	if ((res1 == 1) && (res2 == 1)) { // найдено 2 пересечения
		kapi::ksMathPointParamPtr point1 = kompas->GetParamStruct(kapi::ko_MathPointParam);
		dynArr1->ksGetArrayItem(0, point1);
		kapi::ksMathPointParamPtr point2 = kompas->GetParamStruct(kapi::ko_MathPointParam);
		dynArr2->ksGetArrayItem(0, point2);

		kapi::ILineSegmentPtr newLineSegment(lineSegments->Add());
		newLineSegment->X1 = point1->x; newLineSegment->Y1 = point1->y;
		newLineSegment->X2 = point2->x; newLineSegment->Y2 = point2->y;
		newLineSegment->Update();
		ConstraintsCreator constrCreator = ConstraintsCreator(newLineSegment);
		constrCreator.pointOnCurve(0, lineSegment);
		constrCreator.pointOnCurve(1, lineSegment);
		constrCreator.pointOnCurve(0, info.line1);
		constrCreator.pointOnCurve(1, info.line2);

		info.points1.push_back(MergePointInfo{point1->x, point1->y, newLineSegment, 0});
		info.points2.push_back(MergePointInfo{point2->x, point2->y, newLineSegment, 1});
	} else { // найдено одно пересечение
		long partnerIndex = 0; // индекс на опорном отрезке
		double x = 0.0, y = 0.0;
		if (pointInsideInterval(math2d, lineSegment->X1, lineSegment->Y1, info.line1, info.line2)) {
			x = lineSegment->X1; y = lineSegment->Y1;
			partnerIndex = 0;
		} else {
			x = lineSegment->X2; y = lineSegment->Y2;
			partnerIndex = 1;
		}

		kapi::ksMathPointParamPtr point = kompas->GetParamStruct(kapi::ko_MathPointParam);
		if (res1 == 1) {
			dynArr1->ksGetArrayItem(0, point);
		} else {
			dynArr2->ksGetArrayItem(0, point);
		}

		kapi::ILineSegmentPtr newLineSegment(lineSegments->Add());
		newLineSegment->X1 = x; newLineSegment->Y1 = y;
		newLineSegment->X2 = point->x; newLineSegment->Y2 = point->y;
		newLineSegment->Update();

		ConstraintsCreator constrCreator = ConstraintsCreator(newLineSegment);
		constrCreator.mergePoints(0, lineSegment, partnerIndex);
		constrCreator.pointOnCurve(1, lineSegment);
		if (res1 == 1) {
			constrCreator.pointOnCurve(1, info.line1);
			info.points1.push_back(MergePointInfo{point->x, point->y, newLineSegment, 1});
		} else {
			constrCreator.pointOnCurve(1, info.line2);
			info.points2.push_back(MergePointInfo{point->x, point->y, newLineSegment, 1});
		}
	}
}

void processArc(kapi::KompasObjectPtr kompas, Sketch1NotCircleInfo info, kapi::IArcPtr arc) {
	kapi::ksMathematic2DPtr math2d = kompas->GetMathematic2D();

	kapi::ksDynamicArrayPtr dynArr1(kompas->GetDynamicArray(2));
	kapi::ksDynamicArrayPtr dynArr2(kompas->GetDynamicArray(2));
	int res1 = math2d->ksIntersectCurvCurv(arc->Reference, info.line1->Reference, dynArr1);
	int res2 = math2d->ksIntersectCurvCurv(arc->Reference, info.line2->Reference, dynArr2);

	bool arcPoint1InsideInterval = pointInsideInterval(math2d, arc->X1, arc->Y1, info.line1, info.line2);
	bool arcPoint2InsideInterval = pointInsideInterval(math2d, arc->X2, arc->Y2, info.line1, info.line2);

	if (arcPoint1InsideInterval && arcPoint2InsideInterval) { // обе точки внутри промежутка
		if (res1 + res2 == 0) { // нет пересечений
			return;
		}
		if (res1 + res2 == 1) {
			// одно пересечение с одной осью
			if (res1 == 1 && dynArr1->ksGetArrayCount() == 1) {
				return;
			}
			if (res2 == 1 && dynArr2->ksGetArrayCount() == 1) {
				return;
			}
		}
	}

	arc->Style = kapi::ksCurveStyleEnum::ksCSThin;
	arc->Update();

	kapi::IArcsPtr arcs(info.sketch.drawingContainer->Arcs);

	if ((res1 + res2 == 1)) { // пересечени(е)/(я) только с одной осью
		kapi::ksDynamicArrayPtr dynArr;
		
		if (dynArr1->ksGetArrayCount() != 0) { dynArr = dynArr1; } else { dynArr = dynArr2; }

		if (dynArr->ksGetArrayCount() == 1) { // всего одно пересечение с одной из осей
			kapi::ksMathPointParamPtr point = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr->ksGetArrayItem(0, point);

			kapi::IArcPtr newArc(arcs->Add());
			newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
			newArc->X1 = arc->X1; newArc->Y1 = arc->Y1;
			newArc->X2 = point->x; newArc->Y2 = point->y;
			newArc->Update();

			ConstraintsCreator c(newArc);
			c.mergePoints(0, arc, 0);
			c.mergePoints(1, arc, 1);
			if (res1 == 1) {
				c.pointOnCurve(2, info.line1);
				info.points1.push_back(MergePointInfo{point->x, point->y, newArc, 2});
			} else {
				c.pointOnCurve(2, info.line2);
				info.points2.push_back(MergePointInfo{point->x, point->y, newArc, 2});
			}
		} else { // два пересечения с одной осью
			kapi::ksMathPointParamPtr point1 = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr->ksGetArrayItem(0, point1);
			kapi::ksMathPointParamPtr point2 = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr->ksGetArrayItem(1, point2);

			double distance1 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point1->x, point1->y);
			double distance2 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point2->x, point2->y);
			if (distance2 < distance1) {
				std::swap(point1, point2);
			}

			if (arcPoint1InsideInterval) { // обе точки внутри промежутка
				// оставляем 2 крайние части дуги
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = arc->X1; newArc->Y1 = arc->Y1;
					newArc->X2 = point1->x; newArc->Y2 = point1->y;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.mergePoints(1, arc, 1);
					if (res1 == 1) {
						c.pointOnCurve(2, info.line1);
						info.points1.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
					} else {
						c.pointOnCurve(2, info.line2);
						info.points2.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
					}
				}
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = point2->x; newArc->Y1 = point2->y;
					newArc->X2 = arc->X2; newArc->Y2 = arc->Y2;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.mergePoints(2, arc, 2);
					if (res1 == 1) {
						c.pointOnCurve(1, info.line1);
						info.points1.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
					} else {
						c.pointOnCurve(1, info.line2);
						info.points2.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
					}
				}
			} else { // обе точки вне промежутка
				// оставляем центральную часть дуги
				kapi::IArcPtr newArc(arcs->Add());
				newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
				newArc->X1 = point1->x; newArc->Y1 = point1->y;
				newArc->X2 = point2->x; newArc->Y2 = point2->y;
				newArc->Update();

				ConstraintsCreator c(newArc);
				c.mergePoints(0, arc, 0);
				c.equalRadius(arc);
				if (res1 == 1) {
					c.pointOnCurve(1, info.line1);
					c.pointOnCurve(2, info.line1);
					info.points1.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
					info.points1.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
				} else {
					c.pointOnCurve(1, info.line2);
					c.pointOnCurve(2, info.line2);
					info.points2.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
					info.points2.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
				}
			}
		}
	} else { // пересечения с обеими осями
		if ((dynArr1->ksGetArrayCount() == 1) && (dynArr2->ksGetArrayCount() == 1)) { // одно пересечение с каждой осью
			kapi::ksMathPointParamPtr point1 = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr1->ksGetArrayItem(0, point1);
			kapi::ksMathPointParamPtr point2 = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr2->ksGetArrayItem(0, point2);

			kapi::IArcPtr newArc(arcs->Add());
			newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;

			double distance1 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point1->x, point1->y);
			double distance2 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point2->x, point2->y);
			if (distance2 < distance1) {
				std::swap(point1, point2);
			}

			newArc->X1 = point1->x; newArc->Y1 = point1->y;
			newArc->X2 = point2->x; newArc->Y2 = point2->y;
			newArc->Update();

			ConstraintsCreator c(newArc);
			c.mergePoints(0, arc, 0);
			c.equalRadius(arc);
			if (distance2 < distance1) {
				c.pointOnCurve(1, info.line2);
				c.pointOnCurve(2, info.line1);
				info.points1.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
				info.points2.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
			} else {
				c.pointOnCurve(1, info.line1);
				c.pointOnCurve(2, info.line2);
				info.points1.push_back(MergePointInfo{point1->x, point1->y, newArc, 1});
				info.points2.push_back(MergePointInfo{point2->x, point2->y, newArc, 2});
			}
		} else if ((dynArr1->ksGetArrayCount() == 2) && (dynArr2->ksGetArrayCount() == 1) ||
				   (dynArr1->ksGetArrayCount() == 1) && (dynArr2->ksGetArrayCount() == 2))
		{
			std::vector<std::pair<double, kapi::ksMathPointParamPtr>> points;
			for (int i = 0; i < dynArr1->ksGetArrayCount(); i++) {
				kapi::ksMathPointParamPtr point = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr1->ksGetArrayItem(i, point);
				double distance = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point->x, point->y);
				points.push_back(std::make_pair(distance, point));
			}
			for (int i = 0; i < dynArr2->ksGetArrayCount(); i++) {
				kapi::ksMathPointParamPtr point = kompas->GetParamStruct(kapi::ko_MathPointParam); dynArr2->ksGetArrayItem(i, point);
				double distance = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point->x, point->y);
				points.push_back(std::make_pair(distance, point));
			}
			std::sort(points.begin(), points.end());

			if (arcPoint1InsideInterval) {
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = arc->X1; newArc->Y1 = arc->Y1;
					newArc->X2 = points[0].second->x; newArc->Y2 = points[0].second->y;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.mergePoints(1, arc, 1);
					if (dynArr1->ksGetArrayCount() == 2) {
						c.pointOnCurve(2, info.line1);
						info.points1.push_back(MergePointInfo{points[0].second->x, points[0].second->y, newArc, 2});
					} else {
						c.pointOnCurve(2, info.line2);
						info.points2.push_back(MergePointInfo{points[0].second->x, points[0].second->y, newArc, 2});
					}
				}
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = points[1].second->x; newArc->Y1 = points[1].second->y;
					newArc->X2 = points[2].second->x; newArc->Y2 = points[2].second->y;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.equalRadius(arc);
					if (dynArr1->ksGetArrayCount() == 2) {
						c.pointOnCurve(1, info.line1);
						c.pointOnCurve(2, info.line2);
						info.points1.push_back(MergePointInfo{points[1].second->x, points[1].second->y, newArc, 1});
						info.points2.push_back(MergePointInfo{points[2].second->x, points[2].second->y, newArc, 2});
					} else {
						c.pointOnCurve(1, info.line2);
						c.pointOnCurve(2, info.line1);
						info.points2.push_back(MergePointInfo{points[1].second->x, points[1].second->y, newArc, 1});
						info.points1.push_back(MergePointInfo{points[2].second->x, points[2].second->y, newArc, 2});
					}
				}
			} else {
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = points[0].second->x; newArc->Y1 = points[0].second->y;
					newArc->X2 = points[1].second->x; newArc->Y2 = points[1].second->y;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.equalRadius(arc);
					if (dynArr1->ksGetArrayCount() == 2) {
						c.pointOnCurve(1, info.line2);
						c.pointOnCurve(2, info.line1);
						info.points2.push_back(MergePointInfo{points[0].second->x, points[0].second->y, newArc, 1});
						info.points1.push_back(MergePointInfo{points[1].second->x, points[1].second->y, newArc, 2});
					} else {
						c.pointOnCurve(1, info.line1);
						c.pointOnCurve(2, info.line2);
						info.points1.push_back(MergePointInfo{points[0].second->x, points[0].second->y, newArc, 1});
						info.points2.push_back(MergePointInfo{points[1].second->x, points[1].second->y, newArc, 2});
					}
				}
				{
					kapi::IArcPtr newArc(arcs->Add());
					newArc->Xc = arc->Xc; newArc->Yc = arc->Yc; newArc->Radius = arc->Radius; newArc->Direction = arc->Direction;
					newArc->X1 = points[2].second->x; newArc->Y1 = points[2].second->y;
					newArc->X2 = arc->X2; newArc->Y2 = arc->Y2;
					newArc->Update();

					ConstraintsCreator c(newArc);
					c.mergePoints(0, arc, 0);
					c.mergePoints(2, arc, 2);
					if (dynArr1->ksGetArrayCount() == 2) {
						c.pointOnCurve(1, info.line1);
						info.points1.push_back(MergePointInfo{points[2].second->x, points[2].second->y, newArc, 1});
					} else {
						c.pointOnCurve(1, info.line2);
						info.points2.push_back(MergePointInfo{points[2].second->x, points[2].second->y, newArc, 1});
					}
				}
			}
		}
	}
}

void bridgeHoleBuildNotCircleDrawSketch1(kapi::KompasObjectPtr kompas, Sketch sketch, kapi::ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
	drawLoopProjection(sketch.definition, target.outerLoop);

	double y1 = innerCircle->Yc - innerCircle->Radius;
	double y2 = innerCircle->Yc + innerCircle->Radius;

	// todo: определить оптимальный угол наклона вспомогательных прямых
	kapi::ksSurfacePtr sketchSurface = sketch.definition->GetSurface();
	

	// Строим вспомогательные линии
	std::pair<kapi::ILinePtr, kapi::ILinePtr> basicLines = drawBasicLines(sketch, innerCircle, 40.0);

	kapi::ksMathematic2DPtr math2d = kompas->GetMathematic2D();

	// Точки для замыкания контура
	std::list<MergePointInfo> points1;
	std::list<MergePointInfo> points2;

	Sketch1NotCircleInfo info{sketch, basicLines.first, basicLines.second, points1, points2};

	kapi::ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);
	int nLineSegments = lineSegments->Count;
	for (int iLineSegment = 0; iLineSegment < nLineSegments; iLineSegment++) {
		kapi::ILineSegmentPtr lineSegment(lineSegments->GetLineSegment(iLineSegment));
		processLineSegment(kompas, info, lineSegment);
	}

	kapi::IArcsPtr arcs(sketch.drawingContainer->Arcs);
	int nArcs = arcs->Count;
	for (int iArc = 0; iArc < nArcs; iArc++) {
		kapi::IArcPtr arc(arcs->GetArc(iArc));
		processArc(kompas, info, arc);
	}

    closeContour(lineSegments, points1);
    closeContour(lineSegments, points2);
}

void bridgeHoleBuildDrawSketch2(kapi::KompasObjectPtr kompas, Sketch sketch, BridgeHoleBuildTarget target, int angleCount) {
	kapi::ICirclePtr innerCircle = drawThinInnerCircleProjection(sketch, target);

	kapi::IRegularPolygonsPtr regularPolygons = sketch.drawingContainer->RegularPolygons;
	kapi::IRegularPolygonPtr regularPolygon = regularPolygons->Add();
	regularPolygon->Xc = innerCircle->Xc; regularPolygon->Yc = innerCircle->Yc;
	regularPolygon->Count = angleCount;
	regularPolygon->Describe = true;
	regularPolygon->Radius = innerCircle->Radius;
	regularPolygon->Style = 1;
	regularPolygon->Angle = 0;
	regularPolygon->Update();
	ConstraintsCreator constrCreator(regularPolygon);
	constrCreator.tangentTwoCurves(innerCircle);
	constrCreator.mergePoints(0, innerCircle, 0);
	constrCreator.horizontalAlignPoints(1, regularPolygon, 2);
}

kapi::ksEntityPtr buildBridgeHoles(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets, Settings& settings) {
	Macro macro(part, MACRO_NAME_BRIDGE_HOLE_BUILD, true);

	for (BridgeHoleBuildTarget target : bridgeHoleBuildTargets) {
		Macro macroElement(part, MACRO_NAME_BRIDGE_HOLE_BUILD_ELEMENT, true);

		Sketch sketch(kompas, part, target.face);
		kapi::ICirclePtr innerCircle = drawThinInnerCircleProjection(sketch, target);
		double centerX = innerCircle->Xc, centerY = innerCircle->Yc, radius = innerCircle->Radius;
		
		if (loopIsCircle(target.outerLoop)) {
			bridgeHoleBuildCircleDrawSketch1(sketch, innerCircle, target);
		} else {
			bridgeHoleBuildNotCircleDrawSketch1(kompas, sketch, innerCircle, target);
		}
		sketch.endEdit();
		macroElement.add(sketch.entity);

		Sketch sketch2(kompas, part, target.face);
		bridgeHoleBuildDrawSketch2(kompas, sketch2, target, 4);
		sketch2.endEdit();
		macroElement.add(sketch2.entity);

		Sketch sketch3(kompas, part, target.face);
		bridgeHoleBuildDrawSketch2(kompas, sketch3, target, 8);
		sketch3.endEdit();
		macroElement.add(sketch3.entity);

		macroElement.add(cutExtrusion(part, sketch.entity, true, settings, 1));
		macroElement.add(cutExtrusion(part, sketch2.entity, true, settings, 2));
		macroElement.add(cutExtrusion(part, sketch3.entity, true, settings, 3));
		macro.add(macroElement);
	}
	return macro.getEntity();
}

kapi::ksEntityPtr optimizeBridgeHoleBuild(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, Settings& settings) {
	std::list<BridgeHoleBuildTarget> targets = getBridgeHoleBuildTargets(document3d, part, settings.getPrintSurface()->face);
	if (targets.empty()) {
		return nullptr;
	}
	return buildBridgeHoles(kompas, part, targets, settings);
}
