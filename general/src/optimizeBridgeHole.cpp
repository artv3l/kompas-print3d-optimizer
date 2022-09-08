#include "stdafx.h"
#include "optimizeBridgeHole.hpp"

#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

#include "utils.hpp"
#include "concaveAngle.hpp"

const char* MACRO_NAME_BRIDGE_HOLE_FILL = "Закрытие нависающих отвертий диафрагмой";
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

        if (!holeFace->IsCylinder()) {
            if (!holeFace->IsPlanar()) {
                return false;
            } else {
                measurer->SetObject1(printFace);
                measurer->SetObject2(holeFace);
                measurer->Calc();
                double angle = measurer->angle;
                if (!(doubleEqual(angle, 90.0) || doubleEqual(angle, 270.0))) {
                    return false;
                }
            }
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
                bridgeHoleFillTargets.push_back(BridgeHoleFillTarget{ innerLoop, face });
            }
        }
    }
    return bridgeHoleFillTargets;
}

void fillBridgeHoles(ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleFillTargets, double extrusionDepth) {

    ksEntityPtr macroEntity(part->NewEntity(o3d_MacroObject));
    ksMacro3DDefinitionPtr macro(macroEntity->GetDefinition());
    macroEntity->name = MACRO_NAME_BRIDGE_HOLE_FILL;
    macro->StaffVisible = true;
    macroEntity->Create();

    for (BridgeHoleFillTarget target : bridgeHoleFillTargets) {
        ksEntityPtr sketchEntity(part->NewEntity(o3d_sketch));
        ksSketchDefinitionPtr sketchDef(sketchEntity->GetDefinition());
        sketchDef->SetPlane(target.face);
        sketchEntity->Create();

        macro->Add(sketchEntity);

        sketchDef->BeginEdit();
        ksEdgeCollectionPtr edges(target.loop->EdgeCollection());
        int edgesCount = edges->GetCount();
        for (int edgeIndex = 0; edgeIndex < edgesCount; edgeIndex++) {
            sketchDef->AddProjectionOf(edges->GetByIndex(edgeIndex));
        }
        sketchDef->EndEdit();

        ksEntityPtr extrusionEntity(part->NewEntity(o3d_bossExtrusion));
        ksBossExtrusionDefinitionPtr extrusionDef(extrusionEntity->GetDefinition());
        extrusionDef->chooseType = ksChBodiesAndParts;
        extrusionDef->directionType = dtReverse;
        extrusionDef->SetSideParam(false, etBlind, extrusionDepth, 0, false);
        extrusionDef->SetSketch(sketchEntity);
        extrusionEntity->Create();

        macro->Add(extrusionEntity);
    }
    macroEntity->Update();
}

void optimizeBridgeHoleFill(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, double extrusionDepth, HoleType holeType) {
    std::list<BridgeHoleFillTarget> targets = getBridgeHoleFillTargets(document3d, part, printFace, holeType);
    fillBridgeHoles(part, targets, extrusionDepth);
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
            bridgeHoleBuildTargets.push_back(BridgeHoleBuildTarget{ innerLoop, outerLoop, face });
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

ICirclePtr drawThinCircleProjection(Sketch sketch, ksPartPtr part, BridgeHoleBuildTarget target) {

    IViewsAndLayersManagerPtr viewsAndLayersManager(sketch.document2d_api7->ViewsAndLayersManager);
    IViewsPtr views(viewsAndLayersManager->Views);
    IViewPtr view(views->ActiveView);
    IDrawingContainerPtr drawingContainer(view);

    ksEdgeCollectionPtr innerEdges(target.innerLoop->EdgeCollection());
    ksEdgeDefinitionPtr innerEdge(innerEdges->GetByIndex(0));
    sketch.definition->AddProjectionOf(innerEdge);

    ICirclesPtr circles(drawingContainer->Circles);
    ICirclePtr innerCircle(circles->GetCircle(0));
    innerCircle->Style = ksCurveStyleEnum::ksCSThin;
    innerCircle->Update();

    return innerCircle;
}

void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
    drawLoopProjection(sketch.definition, target.outerLoop);

    IViewsAndLayersManagerPtr viewsAndLayersManager(sketch.document2d_api7->ViewsAndLayersManager);
    IViewsPtr views(viewsAndLayersManager->Views);
    IViewPtr view(views->ActiveView);
    IDrawingContainerPtr drawingContainer(view);

    ICirclesPtr circles(drawingContainer->Circles);
    ICirclePtr outerCircle;
    circles = drawingContainer->Circles;
    for (int circleIndex = 0; circleIndex < circles->Count; circleIndex++) {
        ICirclePtr circle(circles->GetCircle(circleIndex));
        if (circle != innerCircle) {
            outerCircle = circle;
        }
    }
    outerCircle->Style = ksCurveStyleEnum::ksCSThin;
    outerCircle->Update();

    ILineSegmentsPtr lineSegments(drawingContainer->LineSegments);
    ILineSegmentPtr lineSegment1(lineSegments->Add());
    lineSegment1->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment1->Y1 = outerCircle->Yc - innerCircle->Radius;
    lineSegment1->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment1->Y2 = outerCircle->Yc - innerCircle->Radius;
    lineSegment1->Update();
    IDrawingObjectPtr lineSegment1DrawingObject(lineSegment1);
    IDrawingObject1Ptr lineSegment1DrawingObject1(lineSegment1DrawingObject);
    {
        IParametriticConstraintPtr constraint(lineSegment1DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCPointOnCurve;
        constraint->Index = 0;
        constraint->Partner = static_cast<IDispatch*>(outerCircle);
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment1DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCPointOnCurve;
        constraint->Index = 1;
        constraint->Partner = static_cast<IDispatch*>(outerCircle);
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment1DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCHorizontal;
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment1DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCTangentTwoCurves;
        constraint->Partner = static_cast<IDispatch*>(innerCircle);
        constraint->Create();
    }

    ILineSegmentPtr lineSegment2(lineSegments->Add());
    lineSegment2->X1 = outerCircle->Xc + innerCircle->Radius; lineSegment2->Y1 = outerCircle->Yc + innerCircle->Radius;
    lineSegment2->X2 = outerCircle->Xc - innerCircle->Radius; lineSegment2->Y2 = outerCircle->Yc + innerCircle->Radius;
    lineSegment2->Update();
    IDrawingObjectPtr lineSegment2DrawingObject(lineSegment2);
    IDrawingObject1Ptr lineSegment2DrawingObject1(lineSegment2DrawingObject);
    {
        IParametriticConstraintPtr constraint(lineSegment2DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCPointOnCurve;
        constraint->Index = 0;
        constraint->Partner = static_cast<IDispatch*>(outerCircle);
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment2DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCPointOnCurve;
        constraint->Index = 1;
        constraint->Partner = static_cast<IDispatch*>(outerCircle);
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment2DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCParallel;
        constraint->Partner = static_cast<IDispatch*>(lineSegment1);
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(lineSegment2DrawingObject1->NewConstraint());
        constraint->ConstraintType = ksCTangentTwoCurves;
        constraint->Partner = static_cast<IDispatch*>(innerCircle);
        constraint->Create();
    }

    IArcsPtr arcs(drawingContainer->Arcs);
    {
        IArcPtr arc(arcs->Add());
        arc->Xc = outerCircle->Xc; arc->Yc = outerCircle->Yc;
        arc->Radius = outerCircle->Radius;
        arc->X1 = lineSegment1->X1; arc->Y1 = lineSegment1->Y1;
        arc->X2 = lineSegment2->X1; arc->Y2 = lineSegment2->Y1;
        arc->Direction = false;
        arc->Update();

        IDrawingObjectPtr arcDrawingObject(arc);
        IDrawingObject1Ptr arcDrawingObject1(arcDrawingObject);
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 0;
            constraint->Partner = static_cast<IDispatch*>(outerCircle);
            constraint->PartnerIndex = 0;
            constraint->Create();
        }
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 1;
            constraint->Partner = static_cast<IDispatch*>(lineSegment1);
            constraint->PartnerIndex = 0;
            constraint->Create();
        }
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 2;
            constraint->Partner = static_cast<IDispatch*>(lineSegment2);
            constraint->PartnerIndex = 0;
            constraint->Create();
        }
    }
    {
        IArcPtr arc(arcs->Add());
        arc->Xc = outerCircle->Xc; arc->Yc = outerCircle->Yc;
        arc->Radius = outerCircle->Radius;
        arc->X1 = lineSegment1->X2; arc->Y1 = lineSegment1->Y2;
        arc->X2 = lineSegment2->X2; arc->Y2 = lineSegment2->Y2;
        arc->Direction = true;
        arc->Update();

        IDrawingObjectPtr arcDrawingObject(arc);
        IDrawingObject1Ptr arcDrawingObject1(arcDrawingObject);
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 0;
            constraint->Partner = static_cast<IDispatch*>(outerCircle);
            constraint->PartnerIndex = 0;
            constraint->Create();
        }
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 1;
            constraint->Partner = static_cast<IDispatch*>(lineSegment1);
            constraint->PartnerIndex = 1;
            constraint->Create();
        }
        {
            IParametriticConstraintPtr constraint(arcDrawingObject1->NewConstraint());
            constraint->ConstraintType = ksCMergePoints;
            constraint->Index = 2;
            constraint->Partner = static_cast<IDispatch*>(lineSegment2);
            constraint->PartnerIndex = 1;
            constraint->Create();
        }
    }
}

void closeContour(ILineSegmentsPtr lineSegments, std::list<double> points, double y) {
    points.sort();

    // Размеры всегда будут четным
    for (std::list<double>::const_iterator it = points.cbegin(); it != points.cend(); it++) {
        ILineSegmentPtr lineSegment(lineSegments->Add());
        lineSegment->X1 = *it; lineSegment->Y1 = y;
        it++;
        lineSegment->X2 = *it; lineSegment->Y2 = y;
        lineSegment->Update();
    }
}

void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target) {
    drawLoopProjection(sketch.definition, target.outerLoop);

    IViewsAndLayersManagerPtr viewsAndLayersManager(sketch.document2d_api7->ViewsAndLayersManager);
    IViewsPtr views(viewsAndLayersManager->Views);
    IViewPtr view(views->ActiveView);
    IDrawingContainerPtr drawingContainer(view);

    double yMin = innerCircle->Yc - innerCircle->Radius;
    double yMax = innerCircle->Yc + innerCircle->Radius;

    // Строим вспомогательные линии
    ILinesPtr lines(drawingContainer->Lines);

    ILinePtr line1(lines->Add());
    line1->X1 = innerCircle->Xc + 1; line1->Y1 = yMin;
    line1->X2 = innerCircle->Xc - 1; line1->Y2 = yMin;
    line1->Update();

    ILinePtr line2(lines->Add());
    line2->X1 = innerCircle->Xc + 1; line2->Y1 = yMax;
    line2->X2 = innerCircle->Xc - 1; line2->Y2 = yMax;
    line2->Update();

    ksMathematic2DPtr math2d = kompas->GetMathematic2D();

    // Точки для замыкания контура
    std::list<double> pointsMin;
    std::list<double> pointsMax;

    ILineSegmentsPtr lineSegments(drawingContainer->LineSegments);
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
            continue;
        }

        if ((res1 == 1) && (res2 == 1)) {
            ksMathPointParamPtr point1 = kompas->GetParamStruct(ko_MathPointParam);
            dynArr1->ksGetArrayItem(0, point1);
            ksMathPointParamPtr point2 = kompas->GetParamStruct(ko_MathPointParam);
            dynArr2->ksGetArrayItem(0, point2);

            ILineSegmentPtr newLineSegment(lineSegments->Add());
            newLineSegment->X1 = point1->x; newLineSegment->Y1 = point1->y;
            newLineSegment->X2 = point2->x; newLineSegment->Y2 = point2->y;
            newLineSegment->Update();

            pointsMin.push_back(point1->x);
            pointsMax.push_back(point2->x);

            continue;
        }
        
        ksMathPointParamPtr point = kompas->GetParamStruct(ko_MathPointParam);
        if (res1 == 1) {
            dynArr1->ksGetArrayItem(0, point);
            pointsMin.push_back(point->x);
        } else {
            dynArr2->ksGetArrayItem(0, point);
            pointsMax.push_back(point->x);
        }

        ILineSegmentPtr newLineSegment(lineSegments->Add());

        if ((lineSegment->Y1 > yMin) && (lineSegment->Y1 < yMax)) {
            newLineSegment->X1 = lineSegment->X1; newLineSegment->Y1 = lineSegment->Y1;
        } else {
            newLineSegment->X1 = lineSegment->X2; newLineSegment->Y1 = lineSegment->Y2;
        }
        newLineSegment->X2 = point->x; newLineSegment->Y2 = point->y;
        newLineSegment->Update();

    }

    closeContour(lineSegments, pointsMin, yMin);
    closeContour(lineSegments, pointsMax, yMax);
}

void bridgeHoleBuildDrawSketch2(KompasObjectPtr kompas, Sketch sketch, double centerX, double centerY, double radius, int angleCount) {
    ksRegularPolygonParamPtr polygonParam(kompas->GetParamStruct(ko_RegularPolygonParam));
    polygonParam->xc = centerX; polygonParam->yc = centerY;
    polygonParam->count = angleCount;
    polygonParam->describe = true;
    polygonParam->radius = radius;
    polygonParam->style = 1;

    sketch.document2d->ksRegularPolygon(polygonParam, 0);
}

void buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleBuildTargets, double stepDepth) {

    ksEntityPtr macroEntity(part->NewEntity(o3d_MacroObject));
    ksMacro3DDefinitionPtr macro(macroEntity->GetDefinition());
    macroEntity->name = MACRO_NAME_BRIDGE_HOLE_BUILD;
    macro->StaffVisible = true;
    macroEntity->Create();

    for (BridgeHoleBuildTarget target : bridgeHoleBuildTargets) {
        ksEntityPtr macroElementEntity(part->NewEntity(o3d_MacroObject));
        ksMacro3DDefinitionPtr macroElement(macroElementEntity->GetDefinition());
        macroElementEntity->name = MACRO_NAME_BRIDGE_HOLE_BUILD_ELEMENT;
        macroElement->StaffVisible = true;
        macroElementEntity->Create();

        Sketch sketch = createSketch(kompas, part, target.face);
        ICirclePtr innerCircle = drawThinCircleProjection(sketch, part, target);
        double centerX = innerCircle->Xc, centerY = innerCircle->Yc, radius = innerCircle->Radius;

        if (loopIsCircle(target.outerLoop)) {
            bridgeHoleBuildCircleDrawSketch1(sketch, innerCircle, target);
        } else {
            bridgeHoleBuildNotCircleDrawSketch1(kompas, sketch, innerCircle, target);
        }
        sketch.definition->EndEdit();
        macroElement->Add(sketch.entity);
        macroElement->Add(cutExtrusion(part, sketch.entity, true, stepDepth));

        {
            Sketch sketch2 = createSketch(kompas, part, target.face);
            bridgeHoleBuildDrawSketch2(kompas, sketch2, centerX, centerY, radius, 4);
            sketch2.definition->EndEdit();
            macroElement->Add(sketch2.entity);
            macroElement->Add(cutExtrusion(part, sketch2.entity, true, stepDepth * 2));
        }
        {
            Sketch sketch2 = createSketch(kompas, part, target.face);
            bridgeHoleBuildDrawSketch2(kompas, sketch2, centerX, centerY, radius, 8);
            sketch2.definition->EndEdit();
            macroElement->Add(sketch2.entity);
            macroElement->Add(cutExtrusion(part, sketch2.entity, true, stepDepth * 3));
        }
        
        macroElementEntity->Update();
        macro->Add(macroElementEntity);
    }
    macroEntity->Update();
}

void optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, double stepDepth) {
    std::list<BridgeHoleBuildTarget> targets = getBridgeHoleBuildTargets(document3d, part, printFace);
    buildBridgeHoles(kompas, part, targets, stepDepth);
}
