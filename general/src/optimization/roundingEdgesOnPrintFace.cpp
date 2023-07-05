#include "stdafx.h"
#include "optimization/roundingEdgesOnPrintFace.hpp"

#include <string>
#include <utility>
#include <atlbase.h>

#include "utils.hpp"
#include "apiutil/Macro.hpp"
#include "apiutil/ConstraintsCreator.hpp"
#include "apiutil/Sketch.hpp"
#include "settings/PrintSurface.hpp"
#include "settings/DocumentData.hpp"
#include "settings/Setting.hpp"

const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE = "Скругленные ребра на плоскости печати";
const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT = "Контур";
const char* MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT_WITH_REWORK = "Контур - ДОРАБОТКА";

double getCylinderOrTorusRadius(ksFaceDefinitionPtr face) {
    if (face->IsCylinder()) {
        double height = 0.0, radius = 0.0;
        face->GetCylinderParam(&height, &radius);
        return radius;
    } else if (face->IsTorus()) {
        ksSurfacePtr surface(face->GetSurface());
        ksTorusParamPtr torusParam(surface->GetSurfaceParam());
        return torusParam->radius;
    }
    return 0.0;
}

bool faceNeedRework(ksFaceDefinitionPtr roundingFace) {
    if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
        return true;
    }

    ksEdgeCollectionPtr edges(roundingFace->EdgeCollection());
    int edgesCount = edges->GetCount();
    if (roundingFace->IsCylinder()) {
        // Если грань цилиндрическая, то два ребра прямые, а другие два дуги
        if (edgesCount != 4) {
            return true;
        }
        int straightCount = 0, arcCount = 0;
        for (int i = 0; i < edgesCount; i++) {
            ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
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
                ksEdgeDefinitionPtr edge(edges->GetByIndex(i));
                double length = edge->GetLength(ksLengthUnitsEnum::ksLUnMM);
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
    std::list<ksEdgeDefinitionPtr> firstAndLastEdge;
    firstAndLastEdge.push_back(target.trajectory.front()); firstAndLastEdge.push_back(target.trajectory.back());
    for (ksEdgeDefinitionPtr edge : firstAndLastEdge) {
        ksFaceDefinitionPtr roundingFace(edge->GetAdjacentFace(false));
        if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
            roundingFace = edge->GetAdjacentFace(true);
        }
        if (faceNeedRework(roundingFace)) {
            return true;
        }
    }
    return false;
}

std::list<RoundingEdgeOnPrintFaceTarget> getRoundingEdgesOnPrintFaceTargets(ksPartPtr part, PrintSurface printSurface, ReworkType reworkType) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets;

    ksBodyPtr body = part->GetMainBody();
    ksFaceCollectionPtr faces = body->FaceCollection();
    int nFaces = faces->GetCount();
    for (int iFace = 0; iFace < nFaces; iFace++) {
        ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        if (!face->IsPlanar()) {
            continue;
        }
        if ((face != printSurface.face) && (PlaneEq(face) != printSurface.eq)) {
            continue;
        }
        
        ksLoopCollectionPtr loops(face->LoopCollection());
        for (int loopIndex = 0; loopIndex < loops->GetCount(); loopIndex++) {
            ksLoopPtr loop(loops->GetByIndex(loopIndex));

            RoundingEdgeOnPrintFaceTarget target;
            double radius = 0.0;

            bool firstEdgeInTarget = false;
            bool firstTargetInLoopCompleted = false;
            double firstEdgeRadius = 0.0;
            std::list<RoundingEdgeOnPrintFaceTarget>::iterator targetWithFirstEdge;

            ksEdgeCollectionPtr edges(loop->EdgeCollection());
            for (int iEdge = 0; iEdge < edges->GetCount(); iEdge++) {
                ksEdgeDefinitionPtr edge(edges->GetByIndex(iEdge));

                ksFaceDefinitionPtr roundingFace(edge->GetAdjacentFace(false));
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

void drawSketch(Sketch sketch, RoundingEdgeOnPrintFaceTarget target, NumericSetting::Ptr overhangThreshold) {
    std::string temp = "180 - " + overhangThreshold->getExpression();
    _bstr_t expression(temp.c_str());
    
    // Добавляем проекции
    sketch.definition->AddProjectionOf(target.trajectory.front()->GetVertex(true));
    IPointsPtr points(sketch.drawingContainer->Points);
    IPointPtr startPoint(points->GetPoint(0));

    sketch.definition->AddProjectionOf(target.roundingFace);
    IArcsPtr arcs(sketch.drawingContainer->Arcs);
    IArcPtr roundingArc = nullptr;
    bool startPointIs1 = false;
    for (int i = 0; i < arcs->GetCount(); i++) {
        IArcPtr arc(arcs->GetArc(i));
        if (!roundingArc && ((doubleEqual(startPoint->X, arc->X1) && doubleEqual(startPoint->Y, arc->Y1)) ||
                (doubleEqual(startPoint->X, arc->X2) && doubleEqual(startPoint->Y, arc->Y2)))) {
            roundingArc = arc;
            if (doubleEqual(startPoint->X, arc->X1)) {
                startPointIs1 = true;
            }
        }
        arc->Style = ksCurveStyleEnum::ksCSThin;
        arc->Update();
    }
    
    ILineSegmentsPtr lineSegments(sketch.drawingContainer->LineSegments);
    for (int i = 0; i < lineSegments->GetCount(); i++) {
        ILineSegmentPtr lineSegment(lineSegments->GetLineSegment(i));
        lineSegment->Style = ksCurveStyleEnum::ksCSThin;
        lineSegment->Update();
    }

    // Строим два отрезка
    ILineSegmentPtr lineSeg1(lineSegments->Add());
    lineSeg1->X1 = startPoint->X; lineSeg1->Y1 = startPoint->Y;
    if (startPointIs1) {
        lineSeg1->X2 = roundingArc->X2; lineSeg1->Y2 = roundingArc->Y2;
    } else {
        lineSeg1->X2 = roundingArc->X1; lineSeg1->Y2 = roundingArc->Y1;
    }
    lineSeg1->Update();
    ConstraintsCreator constrCreator(lineSeg1);
    constrCreator.mergePoints(0, roundingArc, startPointIs1 ? 1 : 2);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.mergePoints(0, startPoint, 0);

    ILineSegmentPtr lineSeg2(lineSegments->Add());
    lineSeg2->X1 = lineSeg1->X2; lineSeg2->Y1 = lineSeg1->Y2;
    if (startPointIs1) {
        lineSeg2->X2 = roundingArc->X2; lineSeg2->Y2 = roundingArc->Y2;
    } else {
        lineSeg2->X2 = roundingArc->X1; lineSeg2->Y2 = roundingArc->Y1;
    }
    lineSeg2->Update();
    constrCreator = ConstraintsCreator(lineSeg2);
    constrCreator.mergePoints(0, lineSeg1, 1);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.pointOnCurve(1, roundingArc);

    IDrawingObjectPtr lineSeg1DrawingObject(lineSeg1);
    IDrawingObjectPtr lineSeg2DrawingObject(lineSeg2);

    // Устанавливаем размеры
    ISymbols2DContainerPtr symbols2dContainer(sketch.view);
    IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);
    
    IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
    angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
    angleDim->BaseObject1 = lineSeg1DrawingObject;
    angleDim->BaseObject2 = lineSeg2DrawingObject;
    angleDim->Radius = 0;
    angleDim->X3 = (lineSeg1->X1 + lineSeg2->X2) / 2;
    angleDim->Y3 = (lineSeg1->Y1 + lineSeg2->Y2) / 2;
    angleDim->Update();
    IDrawingObject1Ptr angleDimDrawingObject1(angleDim);
    {
        IParametriticConstraintPtr constraint(angleDimDrawingObject1->NewConstraint());
        constraint->ConstraintType = ksConstraintTypeEnum::ksCFixedDim;
        constraint->Create();
    }
    {
        IParametriticConstraintPtr constraint(angleDimDrawingObject1->NewConstraint());
        constraint->ConstraintType = ksConstraintTypeEnum::ksCDimWithVariable;
        constraint->Expression = expression;
        constraint->Create();
    }

    // Достраиваем эскиз дугой
    IArcPtr arc(arcs->Add());
    arc->Xc = roundingArc->Xc; arc->Yc = roundingArc->Yc;
    arc->X1 = startPoint->X; arc->Y1 = startPoint->Y;
    arc->X2 = lineSeg2->X2; arc->Y2 = lineSeg2->Y2;
    arc->Radius = roundingArc->Radius;
    if (startPointIs1) {
        arc->Direction = roundingArc->Direction;
    } else {
        arc->Direction = ~roundingArc->Direction;
    }
    arc->Update();
    constrCreator = ConstraintsCreator(arc);
    constrCreator.mergePoints(1, lineSeg1, 0);
    constrCreator.mergePoints(2, lineSeg2, 1);
    constrCreator.equalRadius(roundingArc);
}

std::pair<size_t, ksEntityPtr> optimizeRoundingEdgesOnPrintFace(KompasObjectPtr kompas, ksPartPtr part, DocumentData::Settings& settings, ReworkType reworkType) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets = getRoundingEdgesOnPrintFaceTargets(part, settings.getPrintSurface(), reworkType);
    if (targets.empty()) {
        return std::make_pair(0, nullptr);
    }
    Macro macro(part, MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE, true);

    for (RoundingEdgeOnPrintFaceTarget target : targets) {
        Macro macroElement(part,
            target.needRework ? MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT_WITH_REWORK : MACRO_NAME_ROUNDING_EDGES_ON_PRINT_FACE_ELEMENT,
            true);

        // Создаем плоскость для эскиза
        ksEntityPtr sketchPlane(part->NewEntity(Obj3dType::o3d_planePerpendicular));
        ksPlanePerpendicularDefinitionPtr sketchPlaneDef(sketchPlane->GetDefinition());
        sketchPlaneDef->SetEdge(target.trajectory.front());
        sketchPlaneDef->SetPoint(target.trajectory.front()->GetVertex(true));
        sketchPlane->hidden = true;
        sketchPlane->Create();
        macroElement.add(sketchPlane);
        
        // Создаем эскиз
        Sketch sketch(kompas, part, sketchPlane);
        drawSketch(sketch, target, settings.getNumericSetting(SI_OVERHANG_THRESHOLD.name));
        sketch.definition->EndEdit();
        macroElement.add(sketch.entity);
        
        // Протягиваем эскиз по траектории
        ksEntityPtr evolutionEntity(part->NewEntity(Obj3dType::o3d_bossEvolution));
        ksBossEvolutionDefinitionPtr evolutionDef(evolutionEntity->GetDefinition());
        evolutionDef->chooseType = ksChooseType::ksChBodiesAndParts;
        evolutionDef->sketchShiftType = 1;
        evolutionDef->SetSketch(sketch.entity);
        ksEntityCollectionPtr trajectory(evolutionDef->PathPartArray());
        for (ksEdgeDefinitionPtr edge : target.trajectory) {
            trajectory->Add(edge);
        }
        evolutionEntity->Create();
        macroElement.add(evolutionEntity);
        macro.add(macroElement);
    }
    return std::make_pair(targets.size(), macro.getEntity());
}
