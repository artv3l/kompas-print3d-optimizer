#include "stdafx.h"
#include "optimization/circleHorizontalHoles.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <list>
#include <sstream>
#include <utility>

#include "SettingsManager.hpp"
#include "apiutil/Macro.hpp"
#include "apiutil/Sketch.hpp"
#include "apiutil/ConstraintsCreator.hpp"
#include "utils.hpp"
#include "LinAlg.hpp"

const char* MACRO_NAME_CIRCLE_HORIZONTAL_HOLES = "Горизонтальные круглые отверстия";
const char* MACRO_NAME_CIRCLE_HORIZONTAL_HOLES_ELEMENT = "Объекты построения";
const double RADIUS_RATIO = 1.0 / 3.0;

ksEntityPtr createConeFaceAxis(ksPartPtr part, ksFaceDefinitionPtr coneFace, bool hidden) {
    ksEntityPtr entity = part->NewEntity(Obj3dType::o3d_axisConeFace);
    ksAxisConefaceDefinitionPtr axis = entity->GetDefinition();
    axis->SetFace(coneFace);
    entity->hidden = hidden;
    entity->Create();
    return entity;
}

ksEntityPtr createPlanePerpendicular(ksPartPtr part, ksEntityPtr axis, ksEntityPtr point, bool hidden) {
    ksEntityPtr entity(part->NewEntity(Obj3dType::o3d_planePerpendicular));
    ksPlanePerpendicularDefinitionPtr definition(entity->GetDefinition());
    definition->SetPoint(point);
    definition->SetEdge(axis);
    entity->hidden = hidden;
    entity->Create();
    return entity;
}

ksEntityPtr createPlaneLineToPlane(ksPartPtr part, ksEntityPtr line, ksEntityPtr plane, bool isParallel, bool hidden) {
    ksEntityPtr entity(part->NewEntity(Obj3dType::o3d_planeLineToPlane));
    ksPlaneLineToPlaneDefinitionPtr definition(entity->GetDefinition());
    bool a2 = definition->SetEdge(line);
    bool a1 = definition->SetPlane(plane);
    definition->parallel = isParallel;
    entity->hidden = hidden;
    bool a = entity->Create();
    return entity;
}

IPoint3DPtr createPointCenter(IPart7Ptr part7, IFacePtr face7, bool hidden) {
    IModelContainerPtr modelContainer(part7);

    IPoints3DPtr points3d(modelContainer->Points3D);
    IPoint3DPtr point3d(points3d->Add());
    point3d->ParameterType = ksPoint3DTypeEnum::ksPCenter;
    point3d->Hidden = hidden;
    IPoint3DParamCenterPtr point3dParamCenter(point3d->Parameters);

    IModelObjectPtr faceModelObject(face7);
    point3dParamCenter->SetObject(faceModelObject);
    point3d->Update();

    return point3d;
}

ksEntityPtr createCutExtrusion(ksPartPtr part, Sketch sketch) {
    ksEntityPtr entity(part->NewEntity(Obj3dType::o3d_cutExtrusion));
    ksCutExtrusionDefinitionPtr definition(entity->GetDefinition());
    definition->cut = true;
    definition->chooseType = ksChBodiesAndParts;
    definition->directionType = ksDirectionTypeEnum::dtBoth;
    definition->SetSketch(sketch.entity);

    ksExtrusionParamPtr param(definition->ExtrusionParam());
    param->typeNormal = ksEndTypeEnum::etUpToNearSurface;
    param->typeReverse = ksEndTypeEnum::etUpToNearSurface;

    entity->Create();
    return entity;
}

ICirclePtr createBaseCircle(Sketch sketch, ksFaceDefinitionPtr target, _bstr_t& out_radiusVariable) {
    ksEdgeCollectionPtr edges = target->EdgeCollection();
    ksEdgeDefinitionPtr edge = edges->GetByIndex(0);
    sketch.definition->AddProjectionOf(edge);
    ICirclesPtr circles = sketch.drawingContainer->Circles;
    ICirclePtr baseCircle = nullptr;
    if (circles->Count != 0) {
        baseCircle = circles->GetCircle(0); 
    } else {
        baseCircle = circles->Add();

        IArcsPtr arcs = sketch.drawingContainer->Arcs;
        IArcPtr baseArc = arcs->GetArc(0);
        baseArc->Style = ksCurveStyleEnum::ksCSThin;
        baseArc->Update();

        baseCircle->Xc = baseArc->Xc; baseCircle->Yc = baseArc->Yc; baseCircle->Radius = baseArc->Radius;
        baseCircle->Update();

        ConstraintsCreator constrCreator(baseCircle);
        constrCreator.mergePoints(0, baseArc, 0);
        constrCreator.equalRadius(baseArc);
    }
    baseCircle->Style = ksCurveStyleEnum::ksCSThin;
    baseCircle->Update();

    // создаем радиальный размер и получаем имя переменной
    ISymbols2DContainerPtr symbols2dContainer(sketch.view);
    IRadialDimensionsPtr radialDimensions(symbols2dContainer->RadialDimensions);
    IRadialDimensionPtr radialDim(radialDimensions->Add());
    radialDim->BaseObject = baseCircle;
    radialDim->Update();
    IDrawingObject1Ptr radialDimDrawingObject1(radialDim);
    {
        IParametriticConstraintPtr constraint(radialDimDrawingObject1->NewConstraint());
        constraint->ConstraintType = ksConstraintTypeEnum::ksCDimWithVariable;
        constraint->Create();
        out_radiusVariable = constraint->Variable;
    }

    return baseCircle;
}

void drawTriangle(Sketch sketch, ICirclePtr baseCircle, ILinePtr verticalLine, _bstr_t radiusVariable, double overhangThreshold, double rotationOffset) {
    double radius = baseCircle->Radius;
    ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);

    // Считаем координаты треугольников в системе координат, где начало - центр baseCircle, ось y - verticalLine
    double dx = radius * RADIUS_RATIO;
    double y1 = std::sin(std::acos(dx / radius)) * radius;
    double y2 = y1 + (std::tan(degreeToRadian(overhangThreshold)) * dx);
    Vec2d l1(-dx, y1), r1(dx, y1), lr2(0, y2);

    // Через матрицу трансформации преобразуем эти координаты к координатам эскиза
    double angle = std::atan2(verticalLine->Y2 - verticalLine->Y1, verticalLine->X2 - verticalLine->X1) + rotationOffset;
    TransformationMatrix2d matrix(angle, baseCircle->Xc, baseCircle->Yc);
    l1 = matrix * l1; r1 = matrix * r1; lr2 = matrix * lr2;

    // строим отрезки
    ILineSegmentPtr lineSegL(lineSegments->Add());
    lineSegL->X1 = l1.x; lineSegL->Y1 = l1.y;
    lineSegL->X2 = lr2.x; lineSegL->Y2 = lr2.y;
    lineSegL->Update();
    ILineSegmentPtr lineSegR(lineSegments->Add());
    lineSegR->X1 = r1.x; lineSegR->Y1 = r1.y;
    lineSegR->X2 = lr2.x; lineSegR->Y2 = lr2.y;
    lineSegR->Update();
    ConstraintsCreator constrCreator(lineSegL);
    constrCreator.pointOnCurve(0, baseCircle);
    constrCreator.mergePoints(1, lineSegR, 1);
    constrCreator.pointOnCurve(1, verticalLine);
    constrCreator = ConstraintsCreator(lineSegR);
    constrCreator.pointOnCurve(0, baseCircle);
    constrCreator.equalLength(lineSegL);

    // строим дугу
    IArcsPtr arcs(sketch.drawingContainer->Arcs);
    IArcPtr arc(arcs->Add());
    arc->Xc = baseCircle->Xc; arc->Yc = baseCircle->Yc;
    arc->Radius = radius;
    arc->X1 = l1.x; arc->Y1 = l1.y;
    arc->X2 = r1.x; arc->Y2 = r1.y;
    arc->Direction = true;
    arc->Update();
    constrCreator = ConstraintsCreator(arc);
    constrCreator.mergePoints(0, baseCircle, 0);
    constrCreator.mergePoints(1, lineSegL, 0);
    constrCreator.mergePoints(2, lineSegR, 0);

    ISymbols2DContainerPtr symbols2dContainer(sketch.view);
    ILineDimensionsPtr lineDimensions(symbols2dContainer->LineDimensions);
    IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);

    // линейный размер
    ILineDimensionPtr lineDim(lineDimensions->Add());
    lineDim->X1 = l1.x; lineDim->Y1 = l1.y;
    lineDim->X2 = r1.x; lineDim->Y2 = r1.y;
    lineDim->X3 = (l1.x + r1.x) / 2; lineDim->Y3 = (l1.y + r1.y) / 2;
    lineDim->Orientation = ksLineDimensionOrientationEnum::ksLinDParallel;
    lineDim->Update();
    constrCreator = ConstraintsCreator(lineDim);
    constrCreator.mergePoints(0, lineSegL, 0);
    constrCreator.mergePoints(1, lineSegR, 0);
    constrCreator.fixedDim();
    constrCreator.dimWithVariable(radiusVariable + " / 1.5");

    // угловой размер
    double dimAngle = 180.0 - (2.0 * overhangThreshold);
    IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
    angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
    angleDim->BaseObject1 = lineSegL;
    angleDim->BaseObject2 = lineSegR;
    angleDim->Radius = 0;
    angleDim->X3 = (l1.x + r1.x) / 2; angleDim->Y3 = (l1.y + r1.y) / 2;
    if (dimAngle > 90.0) {
        angleDim->DimensionType = ksAngleDimTypeEnum::ksADMaxAngle;
    } else {
        angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
    }
    angleDim->Update();
    constrCreator = ConstraintsCreator(angleDim);
    constrCreator.fixedDim();
    constrCreator.dimWithVariable(_bstr_t(dimAngle));
}

void drawSketch(Sketch sketch, ksFaceDefinitionPtr target, ksEntityPtr verticalPlane, double overhangThreshold) {
    _bstr_t radiusVariable;
    ICirclePtr baseCircle = createBaseCircle(sketch, target, radiusVariable);
    
    sketch.definition->AddProjectionOf(verticalPlane);
    ILinesPtr lines(sketch.drawingContainer->Lines);
    ILinePtr verticalLine(lines->GetLine(0));

    drawTriangle(sketch, baseCircle, verticalLine, radiusVariable, overhangThreshold, -M_PI_2);
    drawTriangle(sketch, baseCircle, verticalLine, radiusVariable, overhangThreshold, M_PI_2);
}

Macro buildHoleTriangle(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, ksFaceDefinitionPtr target, double overhangThreshold) {
    Macro macro(part, MACRO_NAME_CIRCLE_HORIZONTAL_HOLES_ELEMENT, true);

    // ось по цилиндрической поверхности
    ksEntityPtr axis = createConeFaceAxis(part, target);
    macro.add(axis);

    // точка в центре цилиндрической поверхности
    IPart7Ptr part7 = kompas->TransferInterface(part, ksAPITypeEnum::ksAPI7Dual, 0);
    IFacePtr targetFace7 = kompas->TransferInterface(target, ksAPITypeEnum::ksAPI7Dual, 0);
    IPoint3DPtr point7 = createPointCenter(part7, targetFace7);
    // Т.к. macro это API5, а point3d это API7, то добавить точку напрямую мы не можем. Мне показалось, что проще всего найти эту же точку в EntityCollection
    ksEntityPtr point = nullptr;
    ksEntityCollectionPtr entityCollection = part->EntityCollection(Obj3dType::o3d_point3D);
    for (int i = 0; i < entityCollection->GetCount(); i++) {
        ksEntityPtr entity = entityCollection->GetByIndex(i);
        if (entity->name == point7->Name) {
            point = entity;
            break;
        }
    }
    macro.add(point);

    // плоскость перпендикулярно оси через точку - плоскость эскиза
    ksEntityPtr sketchPlane = createPlanePerpendicular(part, axis, point);
    macro.add(sketchPlane);

    // плоскость, перпендикулярная эскизу. При ее проецировании на эскиз получим вертикальную ось
    ksEntityPtr verticalPlane = createPlaneLineToPlane(part, axis, printFace->GetEntity(), false);
    macro.add(verticalPlane);

    // эскиз
    Sketch sketch(kompas, part, sketchPlane);
    drawSketch(sketch, target, verticalPlane, overhangThreshold);
    sketch.endEdit();
    macro.add(sketch.entity);

    ksEntityPtr cutExtrusion = createCutExtrusion(part, sketch);
    macro.add(cutExtrusion);

    return macro;
}

std::list<ksFaceDefinitionPtr> getCircleHorizontalHoleTargets(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace) {
    ksBodyPtr body = part->GetMainBody();
    std::list<ksFaceDefinitionPtr> targets;

    ksFaceCollectionPtr faces = body->FaceCollection();
    for (int iFace = 0; iFace < faces->GetCount(); iFace++) {
        ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        if (!face->IsCylinder()) {
            continue;
        }

        // проверяем, что цилиндрическая поверхность горизонтальна
        ksMeasurerPtr measurer = part->GetMeasurer();
        measurer->SetObject1(printFace);
        measurer->SetObject2(face);
        measurer->Calc();
        double angle = measurer->angle;
        if (!(doubleEqual(angle, 0.0) || doubleEqual(angle, 180.0))) {
            continue;
        }

        // проверяем, что цилиндрическая поверхность является отверстием
        /*
        * Цилиндрическая поверхность замкнута по U. При этом 0 <= u <= 2pi
        *
        * Получаем нормаль для точки (0, vMin). Нормаль смотрит наружу/из детали
        * Строим вектор из точки (0, vMin) в точку (pi, vMin)
        * Если эти 2 вектора указывают в одном направлении (скалярное произведение больше нуля), то поверхность является отверстием
        */
        ksSurfacePtr surface = face->GetSurface();
        double uMin = surface->GetParamUMin(), uMax = surface->GetParamUMax();
        double vMin = surface->GetParamVMin(), vMax = surface->GetParamVMax();

        double xNormal = 0.0, yNormal = 0.0, zNormal = 0.0;
        surface->GetNormal(0, vMin, &xNormal, &yNormal, &zNormal);
        if (!face->normalOrientation) {
            xNormal = -xNormal; yNormal = -yNormal; zNormal = -zNormal;
        }

        double x0 = 0.0, y0 = 0.0, z0 = 0.0;
        surface->GetPoint(0, vMin, &x0, &y0, &z0);

        double x = 0.0, y = 0.0, z = 0.0;
        surface->GetPoint(M_PI, vMin, &x, &y, &z);
        x -= x0; y -= y0; z -= z0;

        double dot = (x * xNormal) + (y * yNormal) + (z * zNormal);
        if (dot > 0) {
            targets.push_back(face);
        }
    }
    return targets;
}

std::pair<size_t, Optional<Macro>> optimizeCircleHorizontalHoles(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, const Settings& settings) {
    std::list<ksFaceDefinitionPtr> targets = getCircleHorizontalHoleTargets(kompas, document3d, part, settings.printSurface.value().face);
    if (targets.empty()) {
        return std::make_pair(0, Optional<Macro>());
    }

    Macro macro(part, MACRO_NAME_CIRCLE_HORIZONTAL_HOLES, true);
    for (ksFaceDefinitionPtr target : targets) {
        macro.add(buildHoleTriangle(kompas, document3d, part, settings.printSurface.value().face, target, settings.overhangThreshold));
    }
    return std::make_pair(targets.size(), macro);
}
