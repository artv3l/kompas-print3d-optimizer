#include "roundingEdgesOnPrintFace.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <atlbase.h>

#include "kapiwrap/Macro.hpp"
#include "kapiwrap/ConstraintsCreator.hpp"
#include "kapiwrap/Sketch.hpp"

#include "settings/PrintSurface.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"
#include "LinAlg.hpp"
#include "utils.hpp"

const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE = "Скругленные ребра на плоскости печати";
const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT = "Контур";
const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT_WITH_REWORK = "Контур - ДОРАБОТКА";

double getCylinderOrTorusRadius(kapi::ksFaceDefinitionPtr face) {
    if (face->IsCylinder()) {
        double height = 0.0, radius = 0.0;
        face->GetCylinderParam(&height, &radius);
        return radius;
    } else if (face->IsTorus()) {
        kapi::ksSurfacePtr surface(face->GetSurface());
        kapi::ksTorusParamPtr torusParam(surface->GetSurfaceParam());
        return torusParam->radius;
    }
    return 0.0;
}

bool faceNeedRework(kapi::ksFaceDefinitionPtr roundingFace) {
    if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
        return true;
    }

    kapi::ksEdgeCollectionPtr edges(roundingFace->EdgeCollection());
    int edgesCount = edges->GetCount();
    if (roundingFace->IsCylinder()) {
        // Если грань цилиндрическая, то два ребра прямые, а другие два дуги
        if (edgesCount != 4) {
            return true;
        }
        int straightCount = 0, arcCount = 0;
        for (int i = 0; i < edgesCount; i++) {
            kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
            if (edge->IsStraight()) {
                straightCount++;
            } else if (edge->IsArc()) {
                arcCount++;
            }
        }
        return !((straightCount == 2) && (arcCount == 2));
    } else {
        // Если грань тороидальная
        if ((edgesCount == 2) || (edgesCount == 3)) {
            // Два или три ребра никогда не потребуют доработки
            // Про грань с тремя ребрами описано далее
            return false;
        } else if (edgesCount == 4) {
            // Если ребра четыре, то они все должны быть дугами
            for (int i = 0; i < edgesCount; i++) {
                kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
                double length = edge->GetLength(kapi::ksLengthUnitsEnum::ksLUnMM);
                // Также проверяем на полюсные ребра, их длина == 0
                if (!edge->IsArc() && !doubleEqual(length, 0.0)) {
                    return true;
                }
            }
            return false;
        }
        return true;
    }
}

bool targetNeedRework(RoundingEdgeOnPrintFaceTarget target) {
    std::list<kapi::ksEdgeDefinitionPtr> firstAndLastEdge;
    firstAndLastEdge.push_back(target.trajectory.front()); firstAndLastEdge.push_back(target.trajectory.back());
    for (kapi::ksEdgeDefinitionPtr edge : firstAndLastEdge) {
        kapi::ksFaceDefinitionPtr roundingFace(edge->GetAdjacentFace(false));
        if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
            roundingFace = edge->GetAdjacentFace(true);
        }
        if (faceNeedRework(roundingFace)) {
            return true;
        }
    }
    return false;
}

std::list<RoundingEdgeOnPrintFaceTarget> getRoundingEdgesOnPrintFaceTargets(kapi::ksPartPtr part, PrintSurface printSurface, ReworkType reworkType) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets;

    kapi::ksBodyPtr body = part->GetMainBody();
    kapi::ksFaceCollectionPtr faces = body->FaceCollection();
    int nFaces = faces->GetCount();
    for (int iFace = 0; iFace < nFaces; iFace++) {
        kapi::ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        if (!face->IsPlanar()) {
            continue;
        }
        if ((face != printSurface.face) && (PlaneEq(face) != printSurface.eq)) {
            continue;
        }
        
        kapi::ksLoopCollectionPtr loops(face->LoopCollection());
        for (int loopIndex = 0; loopIndex < loops->GetCount(); loopIndex++) {
            kapi::ksLoopPtr loop(loops->GetByIndex(loopIndex));

            RoundingEdgeOnPrintFaceTarget target;
            double radius = 0.0;

            bool firstEdgeInTarget = false;
            bool firstTargetInLoopCompleted = false;
            double firstEdgeRadius = 0.0;
            std::list<RoundingEdgeOnPrintFaceTarget>::iterator targetWithFirstEdge;

            kapi::ksEdgeCollectionPtr edges(loop->EdgeCollection());
            for (int iEdge = 0; iEdge < edges->GetCount(); iEdge++) {
                kapi::ksEdgeDefinitionPtr edge(edges->GetByIndex(iEdge));

                kapi::ksFaceDefinitionPtr roundingFace(edge->GetAdjacentFace(false));
                if (roundingFace == face) {
                    roundingFace = edge->GetAdjacentFace(true);
                }

                if ((edge->IsStraight() && roundingFace->IsCylinder()) ||
                    ((edge->IsCircle() || edge->IsArc()) && roundingFace->IsTorus())) {
                    if (target.trajectory.empty()) {
                        radius = getCylinderOrTorusRadius(roundingFace);
                        target.roundingFace = roundingFace;
                    } else if (!doubleEqual(radius, getCylinderOrTorusRadius(roundingFace))) {
                        targets.push_back(target);
                        target = RoundingEdgeOnPrintFaceTarget();

                        if (firstEdgeInTarget && !firstTargetInLoopCompleted) {
                            targetWithFirstEdge = --targets.end();
                        }
                        firstTargetInLoopCompleted = true;
                    }
                    target.trajectory.push_back(edge);

                    if (iEdge == 0) {
                        firstEdgeInTarget = true;
                        firstEdgeRadius = radius;
                    }
                } else if (!target.trajectory.empty()) {
                    targets.push_back(target);
                    target = RoundingEdgeOnPrintFaceTarget();

                    if (firstEdgeInTarget && !firstTargetInLoopCompleted) {
                        targetWithFirstEdge = --targets.end();
                    }
                    firstTargetInLoopCompleted = true;
                }
            }

            if (!target.trajectory.empty()) {
                if (firstEdgeInTarget && firstTargetInLoopCompleted && doubleEqual(firstEdgeRadius, radius)) {
                    RoundingEdgeOnPrintFaceTarget firstTarget = *(targetWithFirstEdge);
                    targets.erase(targetWithFirstEdge);
                    target.trajectory.insert(target.trajectory.cbegin(), firstTarget.trajectory.cbegin(), firstTarget.trajectory.cend());
                    target.roundingFace = firstTarget.roundingFace;
                }
                targets.push_back(target);
            }
        }
    }

    for (std::list<RoundingEdgeOnPrintFaceTarget>::iterator it = targets.begin(); it != targets.end();) {
        if (targetNeedRework(*it)) {
            if (reworkType == ReworkType::ONLY_WITHOUT_REWORK) {
                it = targets.erase(it);
            } else {
                it->needRework = true;
                it++;
            }
        } else {
            if (reworkType == ReworkType::ONLY_WITH_REWORK) {
                it = targets.erase(it);
            } else {
                it->needRework = false;
                it++;
            }
        }
    }

    return targets;
}

void drawSketch(Sketch sketch, RoundingEdgeOnPrintFaceTarget target, DoubleSetting::Ptr overhangThreshold) {
    std::string temp = "180 - " + overhangThreshold->getExpression();
    _bstr_t expression(temp.c_str());
    double dimAngle = 180.0 - overhangThreshold->getValue();
    
    // Добавляем проекции
    sketch.definition->AddProjectionOf(target.trajectory.front()->GetVertex(true));
    kapi::IPointsPtr points(sketch.drawingContainer->Points);
    kapi::IPointPtr startPoint(points->GetPoint(0));
    
    sketch.definition->AddProjectionOf(target.roundingFace);

    // Могут быть проекции-окружности, их просто скрываем
    kapi::ICirclesPtr circles(sketch.drawingContainer->Circles);
    for (int i = 0; i < circles->GetCount(); i++) {
        kapi::ICirclePtr circle(circles->GetCircle(i));
        circle->Style = kapi::ksCurveStyleEnum::ksCSThin;
        circle->Update();
    }

    kapi::IArcsPtr arcs(sketch.drawingContainer->Arcs);
    kapi::IArcPtr roundingArc = nullptr;
    bool startPointIs1 = false;
    for (int i = 0; i < arcs->GetCount(); i++) {
        kapi::IArcPtr arc(arcs->GetArc(i));
        if (!roundingArc && ((doubleEqual(startPoint->X, arc->X1) && doubleEqual(startPoint->Y, arc->Y1)) ||
                             (doubleEqual(startPoint->X, arc->X2) && doubleEqual(startPoint->Y, arc->Y2))
                            )) {
            roundingArc = arc;
            if (doubleEqual(startPoint->X, arc->X1) && doubleEqual(startPoint->Y, arc->Y1)) {
                startPointIs1 = true;
            }
        }
        arc->Style = kapi::ksCurveStyleEnum::ksCSThin;
        arc->Update();
    }
    
    double angle = std::atan2(roundingArc->Yc - startPoint->Y, roundingArc->Xc - startPoint->X) - M_PI_2;
    // Задаем локальную систему координат. Центр - стартовая точка (startPoint). Ось Y направлена к центру окружности

    // Получаем координаты средней точки дуги в локальной системе координат. Важен знак координаты по X 
    TransformationMatrix2d testMatrix(-angle, -startPoint->X, -startPoint->Y);
    Vec2d testPoint = testMatrix* Vec2d(roundingArc->X3, roundingArc->Y3);

    TransformationMatrix2d matrix(angle, startPoint->X, startPoint->Y);
    // Смещение по X точки, где соединятся 2 отрезка - mergePoint
    double xOffset = roundingArc->Radius * std::tan(degreeToRadian(90.0 - (dimAngle / 2.0)));
    Vec2d mergePoint = matrix * Vec2d((testPoint.x > 0) ? xOffset : -xOffset, 0.0);
    
    // Рассчитываем координаты точки для второго отрезка. Эта точка булет находиться на дуге
    double xOffset2 = std::cos(degreeToRadian(dimAngle - 90.0)) * roundingArc->Radius;
    Vec2d pointOnArc = matrix * Vec2d(
        (testPoint.x > 0) ? xOffset2 : -xOffset2,
        roundingArc->Radius - (std::sin(degreeToRadian(dimAngle - 90.0)) * roundingArc->Radius));

    kapi::ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);

    // Строим два отрезка
    kapi::ILineSegmentPtr lineSeg1(lineSegments->Add());
    lineSeg1->X1 = startPoint->X; lineSeg1->Y1 = startPoint->Y;
    lineSeg1->X2 = mergePoint.x; lineSeg1->Y2 = mergePoint.y;
    lineSeg1->Update();
    ConstraintsCreator constrCreator(lineSeg1);
    constrCreator.mergePoints(0, roundingArc, startPointIs1 ? 1 : 2);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.mergePoints(0, startPoint, 0);

    kapi::ILineSegmentPtr lineSeg2(lineSegments->Add());
    lineSeg2->X1 = mergePoint.x; lineSeg2->Y1 = mergePoint.y;
    lineSeg2->X2 = pointOnArc.x; lineSeg2->Y2 = pointOnArc.y;
    lineSeg2->Update();
    constrCreator = ConstraintsCreator(lineSeg2);
    constrCreator.mergePoints(0, lineSeg1, 1);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.pointOnCurve(1, roundingArc);
    
    kapi::IDrawingObjectPtr lineSeg1DrawingObject(lineSeg1);
    kapi::IDrawingObjectPtr lineSeg2DrawingObject(lineSeg2);

    // Устанавливаем размеры
    kapi::ISymbols2DContainerPtr symbols2dContainer(sketch.view);
    kapi::IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);
    
    kapi::IAngleDimensionPtr angleDim(angleDimensions->Add(kapi::DrawingObjectTypeEnum::ksDrADimension));
    angleDim->BaseObject1 = lineSeg1DrawingObject;
    angleDim->BaseObject2 = lineSeg2DrawingObject;
    angleDim->Radius = 0;
    angleDim->X3 = (lineSeg1->X1 + lineSeg2->X2) / 2;
    angleDim->Y3 = (lineSeg1->Y1 + lineSeg2->Y2) / 2;
    angleDim->DimensionType = kapi::ksAngleDimTypeEnum::ksADMaxAngle;
    angleDim->Update();
    kapi::IDrawingObject1Ptr angleDimDrawingObject1(angleDim);
    constrCreator = ConstraintsCreator(angleDimDrawingObject1);
    constrCreator.fixedDim();
    constrCreator.dimWithVariable(expression);

    // Достраиваем эскиз дугой
    kapi::IArcPtr arc(arcs->Add());
    arc->Xc = roundingArc->Xc; arc->Yc = roundingArc->Yc;
    arc->X1 = startPoint->X; arc->Y1 = startPoint->Y;
    arc->X2 = pointOnArc.x; arc->Y2 = pointOnArc.y;
    arc->Radius = roundingArc->Radius;
    if (startPointIs1) {
        arc->Direction = roundingArc->Direction;
    } else {
        arc->Direction = !roundingArc->Direction;
    }
    arc->Update();
    constrCreator = ConstraintsCreator(arc);
    constrCreator.mergePoints(1, lineSeg1, 0);
    constrCreator.mergePoints(2, lineSeg2, 1);
    constrCreator.equalRadius(roundingArc);
}

kapi::ksEntityPtr optimizeRoundingEdgesOnPrintFace(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, Settings& settings, ReworkType reworkType, size_t& reworkCount) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets = getRoundingEdgesOnPrintFaceTargets(part, settings.getPrintSurface(), reworkType);
    if (targets.empty()) {
        return nullptr;
    }
    Macro macro(part, MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE, true);

    for (RoundingEdgeOnPrintFaceTarget target : targets) {
        Macro macroElement(part,
            target.needRework ? MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT_WITH_REWORK : MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT,
            true);

        // Создаем плоскость для эскиза
        kapi::ksEntityPtr sketchPlane(part->NewEntity(kapi::Obj3dType::o3d_planePerpendicular));
        kapi::ksPlanePerpendicularDefinitionPtr sketchPlaneDef(sketchPlane->GetDefinition());
        sketchPlaneDef->SetEdge(target.trajectory.front());
        sketchPlaneDef->SetPoint(target.trajectory.front()->GetVertex(true));
        sketchPlane->hidden = true;
        sketchPlane->Create();
        macroElement.add(sketchPlane);
        
        // Создаем эскиз
        Sketch sketch(kompas, part, sketchPlane);
        drawSketch(sketch, target, settings.getDoubleSetting(si::overhangThreshold.name));
        sketch.definition->EndEdit();
        macroElement.add(sketch.entity);
        
        // Протягиваем эскиз по траектории
        kapi::ksEntityPtr evolutionEntity(part->NewEntity(kapi::Obj3dType::o3d_bossEvolution));
        kapi::ksBossEvolutionDefinitionPtr evolutionDef(evolutionEntity->GetDefinition());
        evolutionDef->chooseType = kapi::ksChooseType::ksChBodiesAndParts;
        evolutionDef->sketchShiftType = 1;
        evolutionDef->SetSketch(sketch.entity);
        kapi::ksEntityCollectionPtr trajectory(evolutionDef->PathPartArray());
        for (kapi::ksEdgeDefinitionPtr edge : target.trajectory) {
            trajectory->Add(edge);
        }
        evolutionEntity->Create();
        macroElement.add(evolutionEntity);
        macro.add(macroElement);

        if (target.needRework) {
            reworkCount++;
        }
    }
    return macro.getEntity();
}
