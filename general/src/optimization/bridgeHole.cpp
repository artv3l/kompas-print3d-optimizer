#include "stdafx.h"
#include "optimization/bridgeHole.hpp"

#include <list>
#include <sstream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include "utils.hpp"
#include "concaveAngle.hpp"
#include "apiutil/Macro.hpp"
#include "apiutil/ConstraintsCreator.hpp"
#include "apiutil/Sketch.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "global.hpp"

const char* MACRO_NAME_BRIDGE_HOLE_FILL = "Закрытие нависающих отвертий диафрагмой";
const char* MACRO_NAME_BRIDGE_HOLE_FILL_ELEMENT = "Отверстие";
const char* MACRO_NAME_BRIDGE_HOLE_BUILD = "Достройка нависающих отверстий";
const char* MACRO_NAME_BRIDGE_HOLE_BUILD_ELEMENT = "Отверстие";

bool loopIsCircle(ksLoopPtr loop) {
	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();
	if (edgesCount != 1) {
		return false;
	}
	ksEdgeDefinitionPtr edge(edges->GetByIndex(0));
	if (!edge->IsCircle()) {
		return false;
	}
	return true;
}

bool checkFaceWithHole(ksFaceDefinitionPtr face, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer) {
	if (!face->IsPlanar() || (face == printFace)) {
		return false;
	}
	measurer->SetObject1(printFace);
	measurer->SetObject2(face);
	measurer->Calc();
	double angle = measurer->angle;
	if (!(doubleEqual(angle, 0.0) || doubleEqual(angle, 180))) {
		return false;
	}
	return true;
}

bool isHoleDirect(ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer) {
	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();
	for (int holeEdgeIndex = 0; holeEdgeIndex < edgesCount; holeEdgeIndex++) {
		ksEdgeDefinitionPtr holeEdge(edges->GetByIndex(holeEdgeIndex));

		ksFaceDefinitionPtr holeFace(holeEdge->GetAdjacentFace(false));
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
		if (!(doubleEqual(angle, 90.0) || doubleEqual(angle, 270.0))) {
			return false;
		}
	}
	return true;
}

bool checkHoleLoop(ksDocument3DPtr document3d, ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer) {
	if (!isHoleDirect(face, loop, printFace, measurer)) {
		return false;
	}

	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	int edgesCount = edges->GetCount();

	for (int holeEdgeIndex = 0; holeEdgeIndex < edgesCount; holeEdgeIndex++) {
		ksEdgeDefinitionPtr holeEdge(edges->GetByIndex(holeEdgeIndex));

		ksFaceDefinitionPtr holeFace(holeEdge->GetAdjacentFace(false));
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

		ksEdgeCollectionPtr edges2(holeFace->EdgeCollection());
		for (int edge2Index = 0; edge2Index < edges2->GetCount(); edge2Index++) {
			ksEdgeDefinitionPtr edge2(edges2->GetByIndex(edge2Index));
			if (edge2 == holeEdge) {
				continue;
			}

			measurer->SetObject1(printFace);
			measurer->SetObject2(edge2);
			measurer->Calc();
			if (edge2->IsStraight()) {
				double angle = measurer->angle;
				if (doubleEqual(angle, 90.0) || doubleEqual(angle, 270.0)) {
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

ksEntityPtr cutExtrusion(ksPartPtr part, ksEntityPtr sketchEntity, bool normalDirection, Settings& settings, int multiplier) {
	ksEntityPtr extrusionEntity(part->NewEntity(o3d_cutExtrusion));
	ksCutExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
	extrusionDef->cut = true;
	extrusionDef->chooseType = ksChBodiesAndParts;
	if (normalDirection) {
		extrusionDef->directionType = dtNormal;
	} else {
		extrusionDef->directionType = dtReverse;
	}
	double depth = settings.getNumericSetting(SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT.name)->getValue() *
		settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getValue() * multiplier;
	extrusionDef->SetSideParam(normalDirection, etBlind, depth, 0, false);
	extrusionDef->SetSketch(sketchEntity);
	extrusionEntity->Create();

	{ // Привязываем размеры к переменным
		ksFeaturePtr feature(extrusionEntity->GetFeature());
		ksVariableCollectionPtr variableCollection(feature->VariableCollection);
		ksVariablePtr variable(variableCollection->GetByIndex(1)); // Индекс=3 - "Расстояние 1"

		std::ostringstream oss;
		oss << multiplier << " * ("
			<< settings.getNumericSetting(SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT.name)->getExpression() << " * "
			<< settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getExpression()
			<< ")";
		variable->Expression = oss.str().c_str();
	}

	return extrusionEntity;
}

/* Закрытие нависающих отверстий тонким слоем материала */

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, HoleType holeType) {
	ksMeasurerPtr measurer(part->GetMeasurer());

	ksBodyPtr body = part->GetMainBody();
	ksFaceCollectionPtr faces = body->FaceCollection();
	int facesCount = faces->GetCount();

	std::list<BridgeHoleFillTarget> bridgeHoleFillTargets;

	for (int faceIndex = 0; faceIndex < facesCount; faceIndex++) {
		ksFaceDefinitionPtr face = faces->GetByIndex(faceIndex);
		if (!checkFaceWithHole(face, printFace, measurer)) {
			continue;
		}

		ksLoopCollectionPtr loops(face->LoopCollection());
		for (int loopIndex = 0; loopIndex < loops->GetCount(); loopIndex++) {
			ksLoopPtr innerLoop(loops->GetByIndex(loopIndex));
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

ksEntityPtr fillBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleFillTargets, Settings& settings) {
	Macro macro(part, MACRO_NAME_BRIDGE_HOLE_FILL, true);

	for (BridgeHoleFillTarget target : bridgeHoleFillTargets) {
		Macro macroElement(part, MACRO_NAME_BRIDGE_HOLE_FILL_ELEMENT, true);

		Sketch sketch(kompas, part, target.face);
		macroElement.add(sketch.entity);

		ksEdgeCollectionPtr edges(target.loop->EdgeCollection());
		int edgesCount = edges->GetCount();
		for (int edgeIndex = 0; edgeIndex < edgesCount; edgeIndex++) {
			sketch.definition->AddProjectionOf(edges->GetByIndex(edgeIndex));
		}
		sketch.endEdit();

		double extrusionDepth = settings.getNumericSetting(SI_BRIDGE_HOLE_FILL_LAYERS_COUNT.name)->getValue() * settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getValue();
		ksEntityPtr extrusionEntity(part->NewEntity(o3d_bossExtrusion));
		ksBossExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
		extrusionDef->chooseType = ksChBodiesAndParts;
		extrusionDef->directionType = dtReverse;
		extrusionDef->SetSideParam(false, etBlind, extrusionDepth, 0, false);
		extrusionDef->SetSketch(sketch.entity);
		extrusionEntity->Create();
		{ // Привязываем размеры к переменным
			ksFeaturePtr feature(extrusionEntity->GetFeature());
			ksVariableCollectionPtr variableCollection(feature->VariableCollection);
			ksVariablePtr variable(variableCollection->GetByIndex(3)); // Индекс=3 - "Расстояние 2"

			std::ostringstream oss;
			oss << settings.getNumericSetting(SI_BRIDGE_HOLE_FILL_LAYERS_COUNT.name)->getExpression() << " * " << settings.getNumericSetting(SI_LAYER_HEIGHT.name)->getExpression();
			variable->Expression = oss.str().c_str();
		}
		macroElement.add(extrusionEntity);

		macro.add(macroElement);
	}
	return macro.getEntity();
}

ksEntityPtr optimizeBridgeHoleFill(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, Settings& settings, HoleType holeType) {
	std::list<BridgeHoleFillTarget> targets = getBridgeHoleFillTargets(document3d, part, settings.getPrintSurface().face, holeType);
	if (targets.empty()) {
		return nullptr;
	}
	return fillBridgeHoles(kompas, part, targets, settings);
}

/* Достройка нависающих отверстий для печати мостами */

bool isOuterLoopForBuild(ksLoopPtr loop) {
	if (loopIsCircle(loop)) {
		return true;
	}
	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	for (int edgeIndex = 0; edgeIndex < edges->GetCount(); edgeIndex++) {
		ksEdgeDefinitionPtr edge(edges->GetByIndex(edgeIndex));
		if (!edge->IsStraight() && !edge->IsArc()) {
			return false;
		}
	}
	return true;
}

std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace) {
	ksMeasurerPtr measurer(part->GetMeasurer());

	ksBodyPtr body = part->GetMainBody();
	ksFaceCollectionPtr faces = body->FaceCollection();
	int facesCount = faces->GetCount();

	std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets;

	for (int faceIndex = 0; faceIndex < facesCount; faceIndex++) {
		ksFaceDefinitionPtr face = faces->GetByIndex(faceIndex);
		if (!checkFaceWithHole(face, printFace, measurer)) {
			continue;
		}

		ksLoopCollectionPtr loops(face->LoopCollection());
		int loopsCount = loops->GetCount();

		if (loopsCount != 2) {
			continue;
		}

		ksLoopPtr testLoop(loops->GetByIndex(0));
		ksLoopPtr innerLoop, outerLoop;
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
			ksEdgeCollectionPtr innerEdges(innerLoop->EdgeCollection());
			ksEdgeDefinitionPtr innerEdge(innerEdges->GetByIndex(0));
			double innerRadius = innerEdge->GetLength(ksLengthUnitsEnum::ksLUnMM) / (2 * M_PI);

			ksEdgeCollectionPtr outerEdges(outerLoop->EdgeCollection());
			ksEdgeDefinitionPtr outerEdge(outerEdges->GetByIndex(0));
			double outerRadius = outerEdge->GetLength(ksLengthUnitsEnum::ksLUnMM) / (2 * M_PI);

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

void drawLoopProjection(ksSketchDefinitionPtr sketchDef, ksLoopPtr loop) {
	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	for (int i = 0; i < edges->GetCount(); i++) {
		ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
		sketchDef->AddProjectionOf(edge);
	}
}

ICirclePtr drawThinInnerCircleProjection(Sketch sketch, BridgeHoleBuildTarget target) {
	ksEdgeCollectionPtr innerEdges(target.innerLoop->EdgeCollection());
	ksEdgeDefinitionPtr innerEdge(innerEdges->GetByIndex(0));
	sketch.definition->AddProjectionOf(innerEdge);

	ICirclesPtr circles(sketch.drawingContainer->Circles);
	ICirclePtr innerCircle(circles->GetCircle(0));
	innerCircle->Style = ksCurveStyleEnum::ksCSThin;
	innerCircle->Update();

	return innerCircle;
}

void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
	drawLoopProjection(sketch.definition, target.outerLoop);

	ICirclesPtr circles(sketch.drawingContainer->Circles);
	ICirclePtr outerCircle;
	for (int circleIndex = 0; circleIndex < circles->Count; circleIndex++) {
		ICirclePtr circle(circles->GetCircle(circleIndex));
		if (circle != innerCircle) {
			outerCircle = circle;
		}
	}
	outerCircle->Style = ksCurveStyleEnum::ksCSThin;
	outerCircle->Update();

	ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);

	ILineSegmentPtr lineSegment1(lineSegments->Add());
	lineSegment1->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment1->Y1 = outerCircle->Yc - innerCircle->Radius;
	lineSegment1->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment1->Y2 = outerCircle->Yc - innerCircle->Radius;
	lineSegment1->Update();
	ConstraintsCreator constrCreator(lineSegment1);
	constrCreator.pointOnCurve(0, outerCircle);
	constrCreator.pointOnCurve(1, outerCircle);
	constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);

	ILineSegmentPtr lineSegment2(lineSegments->Add());
	lineSegment2->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment2->Y1 = outerCircle->Yc + innerCircle->Radius;
	lineSegment2->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment2->Y2 = outerCircle->Yc + innerCircle->Radius;
	lineSegment2->Update();
	constrCreator = ConstraintsCreator(lineSegment2);
	constrCreator.pointOnCurve(0, outerCircle);
	constrCreator.pointOnCurve(1, outerCircle);
	constrCreator.parallel(lineSegment1);
	constrCreator.tangentTwoCurves(innerCircle);

	IArcsPtr arcs(sketch.drawingContainer->Arcs);
	{
		IArcPtr arc(arcs->Add());
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
		IArcPtr arc(arcs->Add());
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

void closeContour(ILineSegmentsPtr lineSegments, std::list<MergePointInfo> points) {
	size_t nPoints = points.size();
	if ((nPoints < 2) || (nPoints % 2 != 0)) {
		return;
	}
	points.sort([](const MergePointInfo& lhs, const MergePointInfo& rhs) {
		if (doubleEqual(lhs.y, rhs.y)) {
			return lhs.x < rhs.x;
		}
		return lhs.y < rhs.y;
	});

	// Размеры всегда будут четными
	for (std::list<MergePointInfo>::const_iterator it = points.cbegin(); it != points.cend(); it++) {
		ILineSegmentPtr lineSegment(lineSegments->Add());
		lineSegment->X1 = it->x; lineSegment->Y1 = it->y;
		IDrawingObjectPtr drawingObject1 = it->drawingObject; int drawingObjectIndex1 = it->drawingObjectIndex;
		it++;
		lineSegment->X2 = it->x; lineSegment->Y2 = it->y;
		lineSegment->Update();
		ConstraintsCreator c(lineSegment);
		c.mergePoints(0, drawingObject1, drawingObjectIndex1);
		c.mergePoints(1, it->drawingObject, it->drawingObjectIndex);
	}
}

std::pair<ILinePtr, ILinePtr> drawBasicLines(Sketch sketch, ICirclePtr innerCircle) {
	double y1 = innerCircle->Yc - innerCircle->Radius;
	double y2 = innerCircle->Yc + innerCircle->Radius;

	ILinesPtr lines(sketch.drawingContainer->Lines);

	ILinePtr line1(lines->Add());
	line1->X1 = innerCircle->Xc + 1; line1->Y1 = y1;
	line1->X2 = innerCircle->Xc - 1; line1->Y2 = y1;
	line1->Update();
	ConstraintsCreator constrCreator(line1);
	//constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);

	ILinePtr line2(lines->Add());
	line2->X1 = innerCircle->Xc + 1; line2->Y1 = y2;
	line2->X2 = innerCircle->Xc - 1; line2->Y2 = y2;
	line2->Update();
	constrCreator = ConstraintsCreator(line2);
	constrCreator.parallel(line1);
	constrCreator.tangentTwoCurves(innerCircle);

	return std::make_pair(line1, line2);
}

bool pointInsideInterval(ksMathematic2DPtr math2d, double x, double y, ILinePtr line1, ILinePtr line2) {
	double distance1 = math2d->ksDistancePntLineForPoint(x, y, line1->X1, line1->Y1, line1->X2, line1->Y2);
	double distance2 = math2d->ksDistancePntLineForPoint(x, y, line2->X1, line2->Y1, line2->X2, line2->Y2);
	double intervalLength = math2d->ksDistancePntLineForPoint(line1->X1, line1->Y1, line2->X1, line2->Y1, line2->X2, line2->Y2);
	return doubleEqual(distance1 + distance2, intervalLength);
}

void processLineSegment(Sketch1NotCircleInfo info, ILineSegmentPtr lineSegment) {
	ksMathematic2DPtr math2d = global::kompas->GetMathematic2D();
	
	ksDynamicArrayPtr dynArr1(global::kompas->GetDynamicArray(2));
	ksDynamicArrayPtr dynArr2(global::kompas->GetDynamicArray(2));
	int res1 = math2d->ksIntersectCurvCurv(lineSegment->Reference, info.line1->Reference, dynArr1);
	int res2 = math2d->ksIntersectCurvCurv(lineSegment->Reference, info.line2->Reference, dynArr2);

	// Отрезок полностью вне промежутка
	if (!pointInsideInterval(math2d, lineSegment->X1, lineSegment->Y1, info.line1, info.line2) &&
		!pointInsideInterval(math2d, lineSegment->X2, lineSegment->Y2, info.line1, info.line2) &&
		(res1 == 0) && (res2 == 0))
	{
		lineSegment->Style = ksCurveStyleEnum::ksCSThin;
		lineSegment->Update();
		return;
	}

	// Отрезок полностью внутри промежутка
	if ((res1 != 1) && (res2 != 1)) {
		return;
	}

	lineSegment->Style = ksCurveStyleEnum::ksCSThin;
	lineSegment->Update();

	/*
	  Для всех отрезков(newLineSegment), которые построены на основе отрезков(lineSegment), пересекающих line1(y1) и line2(y2):
	  - Первая точка (index в ограничениях равен 0, координаты при создании: X1 и Y1) лежит на line1,
	  - Вторая точка лежит на line2.
	*/

	ILineSegmentsPtr lineSegments(info.sketch.drawingContainer->LineSegments);
	if ((res1 == 1) && (res2 == 1)) { // найдено 2 пересечения
		ksMathPointParamPtr point1 = global::kompas->GetParamStruct(ko_MathPointParam);
		dynArr1->ksGetArrayItem(0, point1);
		ksMathPointParamPtr point2 = global::kompas->GetParamStruct(ko_MathPointParam);
		dynArr2->ksGetArrayItem(0, point2);

		ILineSegmentPtr newLineSegment(lineSegments->Add());
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
		if ((lineSegment->Y1 > info.line1->Y1) && (lineSegment->Y1 < info.line2->Y1)) {
			x = lineSegment->X1; y = lineSegment->Y1;
			partnerIndex = 0;
		} else {
			x = lineSegment->X2; y = lineSegment->Y2;
			partnerIndex = 1;
		}

		ksMathPointParamPtr point = global::kompas->GetParamStruct(ko_MathPointParam);
		if (res1 == 1) {
			dynArr1->ksGetArrayItem(0, point);
		} else {
			dynArr2->ksGetArrayItem(0, point);
		}

		ILineSegmentPtr newLineSegment(lineSegments->Add());
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

void processArc(Sketch1NotCircleInfo info, IArcPtr arc) {
	// todo: в куче мест код можно переиспользовать и сделать лучше
	ksMathematic2DPtr math2d = global::kompas->GetMathematic2D();

	ksDynamicArrayPtr dynArr1(global::kompas->GetDynamicArray(2));
	ksDynamicArrayPtr dynArr2(global::kompas->GetDynamicArray(2));
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

	arc->Style = ksCurveStyleEnum::ksCSThin;
	arc->Update();

	IArcsPtr arcs(info.sketch.drawingContainer->Arcs);

	if ((res1 + res2 == 1)) { // пересечени(е)/(я) только с одной осью
		ksDynamicArrayPtr dynArr;
		
		if (dynArr1->ksGetArrayCount() != 0) { dynArr = dynArr1; } else { dynArr = dynArr2; }

		if (dynArr->ksGetArrayCount() == 1) { // всего одно пересечение с одной из осей
			ksMathPointParamPtr point = global::kompas->GetParamStruct(ko_MathPointParam); dynArr->ksGetArrayItem(0, point);

			IArcPtr newArc(arcs->Add());
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
			ksMathPointParamPtr point1 = global::kompas->GetParamStruct(ko_MathPointParam); dynArr->ksGetArrayItem(0, point1);
			ksMathPointParamPtr point2 = global::kompas->GetParamStruct(ko_MathPointParam); dynArr->ksGetArrayItem(1, point2);

			double distance1 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point1->x, point1->y);
			double distance2 = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point2->x, point2->y);
			if (distance2 < distance1) {
				std::swap(point1, point2);
			}

			if (arcPoint1InsideInterval) { // обе точки внутри промежутка
				// оставляем 2 крайние части дуги
				{
					IArcPtr newArc(arcs->Add());
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
					IArcPtr newArc(arcs->Add());
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
				IArcPtr newArc(arcs->Add());
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
			ksMathPointParamPtr point1 = global::kompas->GetParamStruct(ko_MathPointParam); dynArr1->ksGetArrayItem(0, point1);
			ksMathPointParamPtr point2 = global::kompas->GetParamStruct(ko_MathPointParam); dynArr2->ksGetArrayItem(0, point2);

			IArcPtr newArc(arcs->Add());
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
			std::vector<std::pair<double, ksMathPointParamPtr>> points;
			for (int i = 0; i < dynArr1->ksGetArrayCount(); i++) {
				ksMathPointParamPtr point = global::kompas->GetParamStruct(ko_MathPointParam); dynArr1->ksGetArrayItem(i, point);
				double distance = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point->x, point->y);
				points.push_back(std::make_pair(distance, point));
			}
			for (int i = 0; i < dynArr2->ksGetArrayCount(); i++) {
				ksMathPointParamPtr point = global::kompas->GetParamStruct(ko_MathPointParam); dynArr2->ksGetArrayItem(i, point);
				double distance = math2d->ksDistancePntPntOnCurve(arc->Reference, arc->X1, arc->Y1, point->x, point->y);
				points.push_back(std::make_pair(distance, point));
			}
			std::sort(points.begin(), points.end());

			if (arcPoint1InsideInterval) {
				{
					IArcPtr newArc(arcs->Add());
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
					IArcPtr newArc(arcs->Add());
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
					IArcPtr newArc(arcs->Add());
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
					IArcPtr newArc(arcs->Add());
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

void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
	drawLoopProjection(sketch.definition, target.outerLoop);

	double y1 = innerCircle->Yc - innerCircle->Radius;
	double y2 = innerCircle->Yc + innerCircle->Radius;

	// todo: определить оптимальный угол наклона вспомогательных прямых

	// Строим вспомогательные линии
	std::pair<ILinePtr, ILinePtr> basicLines = drawBasicLines(sketch, innerCircle);

	ksMathematic2DPtr math2d = kompas->GetMathematic2D();

	// Точки для замыкания контура
	std::list<MergePointInfo> points1;
	std::list<MergePointInfo> points2;

	Sketch1NotCircleInfo info{sketch, basicLines.first, basicLines.second, points1, points2};

	ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);
	int nLineSegments = lineSegments->Count;
	for (int iLineSegment = 0; iLineSegment < nLineSegments; iLineSegment++) {
		ILineSegmentPtr lineSegment(lineSegments->GetLineSegment(iLineSegment));
		processLineSegment(info, lineSegment);
	}

	IArcsPtr arcs(sketch.drawingContainer->Arcs);
	int nArcs = arcs->Count;
	for (int iArc = 0; iArc < nArcs; iArc++) {
		IArcPtr arc(arcs->GetArc(iArc));
		processArc(info, arc);
	}

    closeContour(lineSegments, points1);
    closeContour(lineSegments, points2);
}

void bridgeHoleBuildDrawSketch2(KompasObjectPtr kompas, Sketch sketch, BridgeHoleBuildTarget target, int angleCount) {
	ICirclePtr innerCircle = drawThinInnerCircleProjection(sketch, target);

	IRegularPolygonsPtr regularPolygons = sketch.drawingContainer->RegularPolygons;
	IRegularPolygonPtr regularPolygon = regularPolygons->Add();
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

ksEntityPtr buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets, Settings& settings) {
	Macro macro(part, MACRO_NAME_BRIDGE_HOLE_BUILD, true);

	for (BridgeHoleBuildTarget target : bridgeHoleBuildTargets) {
		Macro macroElement(part, MACRO_NAME_BRIDGE_HOLE_BUILD_ELEMENT, true);

		Sketch sketch(kompas, part, target.face);
		ICirclePtr innerCircle = drawThinInnerCircleProjection(sketch, target);
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

ksEntityPtr optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, Settings& settings) {
	std::list<BridgeHoleBuildTarget> targets = getBridgeHoleBuildTargets(document3d, part, settings.getPrintSurface().face);
	if (targets.empty()) {
		return nullptr;
	}
	return buildBridgeHoles(kompas, part, targets, settings);
}
