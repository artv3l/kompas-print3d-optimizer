#include "stdafx.h"
#include "optimization/bridgeHole.hpp"

#include <list>
#include <utility>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Optional.hpp"
#include "utils.hpp"
#include "concaveAngle.hpp"
#include "apiutil/Macro.hpp"
#include "apiutil/ConstraintsCreator.hpp"
#include "apiutil/Sketch.hpp"
#include "settings/DocumentData.hpp"

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
		bool isHoleEdgeLower = true;

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
				isHoleEdgeLower = false;
				break;
			}
		}
		return isHoleEdgeLower;
	}
	return false;
}

ksEntityPtr cutExtrusion(ksPartPtr part, ksEntityPtr sketchEntity, bool normalDirection, double depth) {
	ksEntityPtr extrusionEntity(part->NewEntity(o3d_cutExtrusion));
	ksCutExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
	extrusionDef->cut = true;
	extrusionDef->chooseType = ksChBodiesAndParts;
	if (normalDirection) {
		extrusionDef->directionType = dtNormal;
	} else {
		extrusionDef->directionType = dtReverse;
	}
	extrusionDef->SetSideParam(normalDirection, etBlind, depth, 0, false);
	extrusionDef->SetSketch(sketchEntity);
	extrusionEntity->Create();
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

Macro fillBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleFillTargets, double extrusionDepth) {
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

		ksEntityPtr extrusionEntity(part->NewEntity(o3d_bossExtrusion));
		ksBossExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
		extrusionDef->chooseType = ksChBodiesAndParts;
		extrusionDef->directionType = dtReverse;
		extrusionDef->SetSideParam(false, etBlind, extrusionDepth, 0, false);
		extrusionDef->SetSketch(sketch.entity);
		extrusionEntity->Create();
		macroElement.add(extrusionEntity);

		macro.add(macroElement);
	}
	return macro;
}

std::pair<size_t, Optional<Macro>> optimizeBridgeHoleFill(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, DocumentData::Settings& settings, HoleType holeType) {
	std::list<BridgeHoleFillTarget> targets = getBridgeHoleFillTargets(document3d, part, settings.getPrintSurface().face, holeType);
	if (targets.empty()) {
		return std::make_pair(0, Optional<Macro>());
	}
	double extrusionDepth = settings.getSetting(SI_BRIDGE_HOLE_FILL_LAYERS_COUNT.variableName)->getValue() * settings.getSetting(SI_LAYER_HEIGHT.variableName)->getValue();
	return std::make_pair(
		targets.size(),
		fillBridgeHoles(kompas, part, targets, extrusionDepth)
	);
}

/* Достройка нависающих отверстий для печати мостами */

bool isOuterLoopForBuild(ksLoopPtr loop) {
	if (loopIsCircle(loop)) {
		return true;
	}
	ksEdgeCollectionPtr edges(loop->EdgeCollection());
	for (int edgeIndex = 0; edgeIndex < edges->GetCount(); edgeIndex++) {
		ksEdgeDefinitionPtr edge(edges->GetByIndex(edgeIndex));
		if (!edge->IsStraight()) {
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

void closeContour(ILineSegmentsPtr lineSegments, std::list<std::pair<double, ILineSegmentPtr>> points, double y, long partnerIndex) {
	points.sort();

	// Размеры всегда будут четными
	for (std::list<std::pair<double, ILineSegmentPtr>>::const_iterator it = points.cbegin(); it != points.cend(); it++) {
		ILineSegmentPtr lineSegment(lineSegments->Add());
		lineSegment->X1 = it->first; lineSegment->Y1 = y;
		ILineSegmentPtr partner1 = it->second;
		it++;
		lineSegment->X2 = it->first; lineSegment->Y2 = y;
		lineSegment->Update();
		ConstraintsCreator c(lineSegment);
		c.mergePoints(0, partner1, partnerIndex);
		c.mergePoints(1, it->second, partnerIndex);
	}
}

void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
	drawLoopProjection(sketch.definition, target.outerLoop);

	double yMin = innerCircle->Yc - innerCircle->Radius;
	double yMax = innerCircle->Yc + innerCircle->Radius;

	// Строим вспомогательные линии
	ILinesPtr lines(sketch.drawingContainer->Lines);

	ILinePtr line1(lines->Add());
	line1->X1 = innerCircle->Xc + 1; line1->Y1 = yMin;
	line1->X2 = innerCircle->Xc - 1; line1->Y2 = yMin;
	line1->Update();
	ConstraintsCreator constrCreator(line1);
	constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);

	ILinePtr line2(lines->Add());
	line2->X1 = innerCircle->Xc + 1; line2->Y1 = yMax;
	line2->X2 = innerCircle->Xc - 1; line2->Y2 = yMax;
	line2->Update();
	constrCreator = ConstraintsCreator(line2);
	constrCreator.horizontal();
	constrCreator.tangentTwoCurves(innerCircle);

	ksMathematic2DPtr math2d = kompas->GetMathematic2D();

	// Точки для замыкания контура
	std::list<std::pair<double, ILineSegmentPtr>> pointsMin;
	std::list<std::pair<double, ILineSegmentPtr>> pointsMax;

	ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);
	int lineSegmentsСount = lineSegments->Count;
	for (int lineSegmentIndex = 0; lineSegmentIndex < lineSegmentsСount; lineSegmentIndex++) {
		ILineSegmentPtr lineSegment(lineSegments->GetLineSegment(lineSegmentIndex));

		// Отрезок полностью вне промежутка
		if (((lineSegment->Y1 <= yMin) && (lineSegment->Y2 <= yMin)) ||
			((lineSegment->Y1 >= yMax) && (lineSegment->Y2 >= yMax))) {
			lineSegment->Style = ksCurveStyleEnum::ksCSThin;
			lineSegment->Update();
			continue;
		}

		ksDynamicArrayPtr dynArr1(kompas->GetDynamicArray(2));
		ksDynamicArrayPtr dynArr2(kompas->GetDynamicArray(2));
		int res1 = math2d->ksIntersectCurvCurv(lineSegment->GetReference(), line1->GetReference(), dynArr1);
		int res2 = math2d->ksIntersectCurvCurv(lineSegment->GetReference(), line2->GetReference(), dynArr2);

		if ((res1 == 1) || (res2 == 1)) {
			lineSegment->Style = ksCurveStyleEnum::ksCSThin;
			lineSegment->Update();
		} else {
			// Отрезок полностью внутри промежутка
			continue;
		}

		/*
		  Для всех отрезков(newLineSegment), которые построены на основе отрезков(lineSegment), пересекающих line1(yMin) и line2(yMax):
		  - Первая точка (index в ограничениях равен 0, координаты при создании: X1 и Y1) лежит на line1,
		  - Вторая точка лежит на line2.
		*/

		if ((res1 == 1) && (res2 == 1)) {
			ksMathPointParamPtr point1 = kompas->GetParamStruct(ko_MathPointParam);
			dynArr1->ksGetArrayItem(0, point1);
			ksMathPointParamPtr point2 = kompas->GetParamStruct(ko_MathPointParam);
			dynArr2->ksGetArrayItem(0, point2);

			ILineSegmentPtr newLineSegment(lineSegments->Add());
			newLineSegment->X1 = point1->x; newLineSegment->Y1 = point1->y;
			newLineSegment->X2 = point2->x; newLineSegment->Y2 = point2->y;
			newLineSegment->Update();
			constrCreator = ConstraintsCreator(newLineSegment);
			constrCreator.pointOnCurve(0, lineSegment);
			constrCreator.pointOnCurve(1, lineSegment);
			constrCreator.pointOnCurve(0, line1);
			constrCreator.pointOnCurve(1, line2);

			pointsMin.push_back(std::make_pair(point1->x, newLineSegment));
			pointsMax.push_back(std::make_pair(point2->x, newLineSegment));

			continue;
		}

		long partnerIndex = 0;
		double x = 0.0, y = 0.0;
		if ((lineSegment->Y1 > yMin) && (lineSegment->Y1 < yMax)) {
			x = lineSegment->X1; y = lineSegment->Y1;
			partnerIndex = 0;
		} else {
			x = lineSegment->X2; y = lineSegment->Y2;
			partnerIndex = 1;
		}

		ILineSegmentPtr newLineSegment(lineSegments->Add());
		ksMathPointParamPtr point = kompas->GetParamStruct(ko_MathPointParam);
		if (res1 == 1) {
			dynArr1->ksGetArrayItem(0, point);
			newLineSegment->X1 = point->x; newLineSegment->Y1 = point->y;
			newLineSegment->X2 = x; newLineSegment->Y2 = y;
		} else {
			dynArr2->ksGetArrayItem(0, point);
			newLineSegment->X1 = x; newLineSegment->Y1 = y;
			newLineSegment->X2 = point->x; newLineSegment->Y2 = point->y;
		}

		newLineSegment->Update();
		constrCreator = ConstraintsCreator(newLineSegment);
		if (res1 == 1) {
			constrCreator.mergePoints(1, lineSegment, partnerIndex);
			constrCreator.pointOnCurve(0, lineSegment);
			constrCreator.pointOnCurve(0, line1);
			pointsMin.push_back(std::make_pair(point->x, newLineSegment));
		} else {
			constrCreator.mergePoints(0, lineSegment, partnerIndex);
			constrCreator.pointOnCurve(1, lineSegment);
			constrCreator.pointOnCurve(1, line2);
			pointsMax.push_back(std::make_pair(point->x, newLineSegment));
		}
	}

	closeContour(lineSegments, pointsMin, yMin, 0);
	closeContour(lineSegments, pointsMax, yMax, 1);
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

Macro buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets, double stepDepth) {
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

		macroElement.add(cutExtrusion(part, sketch.entity, true, stepDepth));
		macroElement.add(cutExtrusion(part, sketch2.entity, true, stepDepth * 2));
		macroElement.add(cutExtrusion(part, sketch3.entity, true, stepDepth * 3));
		macro.add(macroElement);
	}
	return macro;
}

std::pair<size_t, Optional<Macro>> optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, DocumentData::Settings& settings) {
	std::list<BridgeHoleBuildTarget> targets = getBridgeHoleBuildTargets(document3d, part, settings.getPrintSurface().face);
	if (targets.empty()) {
		return std::make_pair(0, Optional<Macro>());
	}
	double stepDepth = settings.getSetting(SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT.variableName)->getValue() * settings.getSetting(SI_LAYER_HEIGHT.variableName)->getValue();
	return std::make_pair(
		targets.size(),
		buildBridgeHoles(kompas, part, targets, stepDepth)
	);
}
