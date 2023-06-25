#include "stdafx.h"
#include "optimization/circleHorizontalHoles.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <list>

#include "SettingsManager.hpp"
#include "apiutil/Macro.hpp"
#include "apiutil/Sketch.hpp"
#include "apiutil/ConstraintsCreator.hpp"
#include "utils.hpp"

const char* MACRO_NAME_CIRCLE_HORIZONTAL_HOLES = "Горизонтальные круглые отверстия";
const char* MACRO_NAME_CIRCLE_HORIZONTAL_HOLES_ELEMENT = "Объекты построения";

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

ICirclesPtr createBaseCircle(Sketch sketch, ksFaceDefinitionPtr target, _bstr_t& out_radiusVariable) {
    ksEdgeCollectionPtr edges = target->EdgeCollection();
    ksEdgeDefinitionPtr edge = edges->GetByIndex(0);
    sketch.definition->AddProjectionOf(edge);
    ICirclesPtr circles = sketch.drawingContainer->Circles;
    ICirclePtr baseCircle = nullptr;
    if (edge->IsArc()) {
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
    } else {
        baseCircle = circles->GetCircle(0);
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

void drawSketchTriangles(Sketch sketch, ksFaceDefinitionPtr target) {
    _bstr_t radiusVariable;
    ICirclePtr baseCircle = createBaseCircle(sketch, target, radiusVariable);


}

Macro buildHoleTriangle(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr target) {
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

    // плоскость перпендикулярно оси через точку
    ksEntityPtr plane = createPlanePerpendicular(part, axis, point);
    macro.add(plane);

    // эскиз
    Sketch sketch(kompas, part, plane);
    drawSketchTriangles(sketch, target);
    sketch.endEdit();
    macro.add(sketch.entity);

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

void optimizeCircleHorizontalHoles(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, const Settings& settings) {
    std::list<ksFaceDefinitionPtr> targets = getCircleHorizontalHoleTargets(kompas, document3d, part, settings.printSurface.value().face);
    Macro macro(part, MACRO_NAME_CIRCLE_HORIZONTAL_HOLES, true);
    for (ksFaceDefinitionPtr target : targets) {
        macro.add(buildHoleTriangle(kompas, document3d, part, target));
    }
}
