#include "optimizations.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <atlbase.h>

#include "generic/math.hpp"
#include "kapiwrap/Macro.hpp"
#include "kapiwrap/ConstraintsCreator.hpp"
#include "kapiwrap/Sketch.hpp"
#include "kapiwrap/3d/body.hpp"
#include "kapiwrap/3d/part.hpp"
#include "kapiwrap/3d/plane.hpp"

#include "settings/PrintSurface.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"
#include "LinAlg.hpp"
#include "resources.hpp"

struct RoundingEdgeOnPrintFaceTarget {
    std::list<ksapi::IEdgePtr> trajectory;
    ksapi::IFacePtr roundingFace;
    bool needRework;
};

double getCylinderOrTorusRadius(ksapi::IFacePtr face) {
    if (face->IsCylinder()) {
        double height = 0.0, radius = 0.0, angle = 0.0;
        face->GetConeParam(height, angle, radius);
        return radius;
    } else if (face->IsTorus()) {
        double radius = 0.0, generatrixRadius = 0.0;
        ksapi::IMathSurface3DPtr surface = face->GetMathSurface();
        surface->GetTorusParam(radius, generatrixRadius);
        return radius;
    }
    return 0.0;
}

bool faceNeedRework(ksapi::IFacePtr roundingFace) {
    if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
        return true;
    }

    auto edges = getEdges(roundingFace);
    const size_t edgesCount = edges.size();
    if (roundingFace->IsCylinder()) {
        // Если грань цилиндрическая, то два ребра прямые, а другие два дуги
        if (edgesCount != 4) {
            return true;
        }
        int straightCount = 0, arcCount = 0;
        for (int i = 0; i < edgesCount; i++) {
            ksapi::IEdgePtr edge = edges[i];
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
                ksapi::IEdgePtr edge = edges[i];
                double length = edge->GetLength(ksLengthUnitsEnum::ksLUnMM);
                // Также проверяем на полюсные ребра, их длина == 0
                if (!edge->IsArc() && !math::equal(length, 0.0)) {
                    return true;
                }
            }
            return false;
        }
        return true;
    }
}

bool targetNeedRework(RoundingEdgeOnPrintFaceTarget target) {
    std::list<ksapi::IEdgePtr> firstAndLastEdge;
    firstAndLastEdge.push_back(target.trajectory.front()); firstAndLastEdge.push_back(target.trajectory.back());
    for (ksapi::IEdgePtr edge : firstAndLastEdge) {
        ksapi::IFacePtr roundingFace(edge->GetAdjacentFace(false));
        if (!roundingFace->IsCylinder() && !roundingFace->IsTorus()) {
            roundingFace = edge->GetAdjacentFace(true);
        }
        if (faceNeedRework(roundingFace)) {
            return true;
        }
    }
    return false;
}

std::list<RoundingEdgeOnPrintFaceTarget> getRoundingEdgesOnPrintFaceTargets(ksapi::IPartPtr part, PrintSurface printSurface, ReworkType reworkType) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets;

    for (ksapi::IFacePtr face : getFaces(part)) {
        if (!face->IsPlanar()) {
            continue;
        }
        if (PlaneEq(face) != printSurface.eq) {
            continue;
        }
        
        for (ksapi::ILoopPtr loop : face->GetLoops()) {
            RoundingEdgeOnPrintFaceTarget target;
            double radius = 0.0;

            bool firstEdgeInTarget = false;
            bool firstTargetInLoopCompleted = false;
            double firstEdgeRadius = 0.0;
            std::list<RoundingEdgeOnPrintFaceTarget>::iterator targetWithFirstEdge;

            auto edges = loop->GetEdges();
            for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge) {
                ksapi::IEdgePtr edge = edges[iEdge];

                ksapi::IFacePtr roundingFace(edge->GetAdjacentFace(false));
                if (roundingFace == face) {
                    roundingFace = edge->GetAdjacentFace(true);
                }

                if ((edge->IsStraight() && roundingFace->IsCylinder()) ||
                    ((edge->IsCircle() || edge->IsArc()) && roundingFace->IsTorus())) {
                    if (target.trajectory.empty()) {
                        radius = getCylinderOrTorusRadius(roundingFace);
                        target.roundingFace = roundingFace;
                    } else if (!math::equal(radius, getCylinderOrTorusRadius(roundingFace))) {
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
                if (firstEdgeInTarget && firstTargetInLoopCompleted && math::equal(firstEdgeRadius, radius)) {
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
    const std::wstring expression = L"180 - " + overhangThreshold->getExpressionW();
    const double dimAngle = 180.0 - overhangThreshold->getValue();
    
    auto editor = sketch.edit();

    // Добавляем проекции
    ksapi::IPointPtr startPoint = editor.addProjectionOf(target.trajectory.front()->GetVertex(true))[0];
    
    editor.addProjectionOf(target.roundingFace);

    // Могут быть проекции-окружности, их просто скрываем
    ksapi::ICirclesPtr circles = editor.getDrawingContainer()->GetCircles();
    for (int i = 0; i < circles->GetCount(); i++) {
        ksapi::ICirclePtr circle = circles->GetCircle(i);
        circle->SetStyle(ksCurveStyleEnum::ksCSThin);
        circle->Update();
    }

    ksapi::IArcsPtr arcs = editor.getDrawingContainer()->GetArcs();
    ksapi::IArcPtr roundingArc = nullptr;
    bool startPointIs1 = false;
    for (int i = 0; i < arcs->GetCount(); i++) {
        ksapi::IArcPtr arc = arcs->GetArc(i);
        if (!roundingArc && ((math::equal(startPoint->GetX(), arc->GetX1()) && math::equal(startPoint->GetY(), arc->GetY1())) ||
                             (math::equal(startPoint->GetX(), arc->GetX2()) && math::equal(startPoint->GetY(), arc->GetY2()))
                            )) {
            roundingArc = arc;
            if (math::equal(startPoint->GetX(), arc->GetX1()) && math::equal(startPoint->GetY(), arc->GetY1())) {
                startPointIs1 = true;
            }
        }
        arc->SetStyle(ksCurveStyleEnum::ksCSThin);
        arc->Update();
    }
    
    double angle = std::atan2(roundingArc->GetYc()- startPoint->GetY(), roundingArc->GetXc()- startPoint->GetX()) - M_PI_2;
    // Задаем локальную систему координат. Центр - стартовая точка (startPoint). Ось Y направлена к центру окружности

    // Получаем координаты средней точки дуги в локальной системе координат. Важен знак координаты по X 
    TransformationMatrix2d testMatrix(-angle, -startPoint->GetX(), -startPoint->GetY());
    Vec2d testPoint = testMatrix* Vec2d(roundingArc->GetX3(), roundingArc->GetY3());

    TransformationMatrix2d matrix(angle, startPoint->GetX(), startPoint->GetY());
    // Смещение по X точки, где соединятся 2 отрезка - mergePoint
    double xOffset = roundingArc->GetRadius() * std::tan(math::toRadians(90.0 - (dimAngle / 2.0)));
    Vec2d mergePoint = matrix * Vec2d((testPoint.x > 0) ? xOffset : -xOffset, 0.0);
    
    // Рассчитываем координаты точки для второго отрезка. Эта точка булет находиться на дуге
    double xOffset2 = std::cos(math::toRadians(dimAngle - 90.0)) * roundingArc->GetRadius();
    Vec2d pointOnArc = matrix * Vec2d(
        (testPoint.x > 0) ? xOffset2 : -xOffset2,
        roundingArc->GetRadius() - (std::sin(math::toRadians(dimAngle - 90.0)) * roundingArc->GetRadius()));

    // Строим два отрезка
    ksapi::ILineSegmentPtr lineSeg1 = editor.addLineSegment(startPoint->GetX(), startPoint->GetY(), mergePoint.x, mergePoint.y);
    ConstraintsCreator constrCreator(lineSeg1);
    constrCreator.mergePoints(0, roundingArc, startPointIs1 ? 1 : 2);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.mergePoints(0, startPoint, 0);

    ksapi::ILineSegmentPtr lineSeg2 = editor.addLineSegment(mergePoint.x, mergePoint.y, pointOnArc.x, pointOnArc.y);
    constrCreator = ConstraintsCreator(lineSeg2);
    constrCreator.mergePoints(0, lineSeg1, 1);
    constrCreator.tangentTwoCurves(roundingArc);
    constrCreator.pointOnCurve(1, roundingArc);

    // Устанавливаем размеры
    ksapi::ISymbols2DContainerPtr symbols2dContainer = editor.getSymbols2DContainer();
    ksapi::IAngleDimensionsPtr angleDimensions = symbols2dContainer->GetAngleDimensions();
    
    ksapi::IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
    angleDim->SetBaseObject1(lineSeg1);
    angleDim->SetBaseObject2(lineSeg2);
    angleDim->SetRadius(0);
    angleDim->SetX3((lineSeg1->GetX1() + lineSeg2->GetX2()) / 2);
    angleDim->SetY3((lineSeg1->GetY1() + lineSeg2->GetY2()) / 2);
    angleDim->SetDimensionType(ksAngleDimTypeEnum::ksADMaxAngle);
    angleDim->Update();
    constrCreator = ConstraintsCreator(angleDim);
    constrCreator.fixedDim();
    constrCreator.dimWithVariable(expression);

    // Достраиваем эскиз дугой
    ksapi::IArcPtr arc = editor.addArc(roundingArc->GetXc(), roundingArc->GetYc(), startPoint->GetX(), startPoint->GetY(), pointOnArc.x, pointOnArc.y,
                  roundingArc->GetRadius(), startPointIs1 ? roundingArc->GetDirection() : !roundingArc->GetDirection());
    constrCreator = ConstraintsCreator(arc);
    constrCreator.mergePoints(1, lineSeg1, 0);
    constrCreator.mergePoints(2, lineSeg2, 1);
    constrCreator.equalRadius(roundingArc);
}

void optimizeRoundingEdgesOnPrintFace(ksapi::IPartPtr part, Settings& settings, ReworkType reworkType, size_t& reworkCount) {
    std::list<RoundingEdgeOnPrintFaceTarget> targets = getRoundingEdgesOnPrintFaceTargets(part, *settings.getPrintSurface(), reworkType);
    if (targets.empty()) {
        return;
    }
    Macro macro(part, resources::c_macroNameRoundingEdgesOnPrintFace, true);

    for (RoundingEdgeOnPrintFaceTarget target : targets) {
        Macro macroElement(part,
            target.needRework ? resources::c_macroNameRoundingEdgesOnPrintFaceElementWithRework : resources::c_macroNameRoundingEdgesOnPrintFaceElement,
            true);

        // Создаем плоскость для эскиза
        auto sketchPlane = createPlanePerpendicular(part, target.trajectory.front(), target.trajectory.front()->GetVertex(true), true /*isHidden*/);
        macroElement.add(sketchPlane);
        
        // Создаем эскиз
        Sketch sketch(part, sketchPlane);
        drawSketch(sketch, target, settings.getDoubleSetting(si::overhangThreshold.name));

        macroElement.add(sketch.getObject());
        
        // Протягиваем эскиз по траектории
        std::vector<ksapi::IModelObjectPtr> edges;
        std::copy(target.trajectory.cbegin(), target.trajectory.cend(), std::back_inserter(edges));
        ksapi::IEvolutionPtr evolution = createEvolution(part, sketch, ksEvolutionShiftSketchTypeEnum::ksEvShiftKeepAngle, edges);

        macroElement.add(evolution);
        macroElement.update();
        macro.add(macroElement);

        if (target.needRework) {
            reworkCount++;
        }
    }
}
