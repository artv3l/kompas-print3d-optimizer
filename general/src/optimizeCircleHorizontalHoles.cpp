#include "stdafx.h"
#include "optimizeCircleHorizontalHoles.hpp"
#include "ConstraintsCreator.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include "utils.hpp"
#include "concaveAngle.hpp"
#include "selectPlane.hpp"

#define EPS_ANGLE 0.001
#define EPS_DISTANCE 0.00001

bool checkAngle(ksDocument3DPtr document3d, ksEdgeDefinitionPtr edge) {
  
    try {
        return isConcaveAngle(document3d, edge);
    } catch (const std::runtime_error&) {
        return true;
    }
}
ksEntityPtr makeAxis(ksPartPtr part, ksEntityPtr face1, ksEntityPtr face2) {
    ksEntityPtr axisEntity(part->NewEntity(o3d_axis2Planes));
    ksAxis2PlanesDefinitionPtr axis(axisEntity->GetDefinition());
    axis->SetPlane(1, face1);
    axis->SetPlane(2, face2);
    axisEntity->hidden = true;

    axisEntity->Create();
    return axisEntity;
}

bool checkPlaneEntities(ksPartPtr part, ksEntityPtr first, ksEntityPtr second) {
    ksMeasurerPtr measurer(part->GetMeasurer());
    measurer->SetObject1(first);
    measurer->SetObject2(second);
    measurer->Calc();
    double angle = measurer->angle;
    double distance = measurer->distance;
    return (abs(angle) < EPS_ANGLE || abs(angle - 180) < EPS_ANGLE) && abs(distance) < EPS_DISTANCE;
}


ksEntityPtr createCute(ksPartPtr part, Sketch sketch, ksFaceDefinitionPtr depthFace1, ksFaceDefinitionPtr depthFace2) {

    ksEntityPtr cutEntity(part->NewEntity(o3d_cutExtrusion));
    ksCutExtrusionDefinitionPtr cut(cutEntity->GetDefinition());
    if (checkPlaneEntities(part, sketch.entity, depthFace1->GetEntity())) { //вытягиваем до depthFace2
        cut->directionType = (short)Direction_Type::dtNormal;
        cut->SetSideParam(true, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(true, depthFace2->GetEntity());
        cut->SetSketch(sketch.entity);
        cutEntity->Create();
    } else if (checkPlaneEntities(part, sketch.entity, depthFace2->GetEntity())) { //вытягиваем до depthFace1
        cut->directionType = (short)Direction_Type::dtNormal;
        cut->SetSideParam(true, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(true, depthFace1->GetEntity());
        cut->SetSketch(sketch.entity);
        cutEntity->Create();
    } else { //вытягиваем до depthFace1 и до depthFace2
        cut->directionType = (short)Direction_Type::dtBoth;
        cut->SetSideParam(false, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(false, depthFace1->GetEntity());
        cut->SetSideParam(true, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(true, depthFace2->GetEntity());
        cut->SetSketch(sketch.entity);
        cutEntity->Create();
    }
    return cutEntity;
}


std::set<ksFaceDefinitionPtr> getHorizontalCircleHoles(ksDocument3DPtr document3d, ksFaceDefinitionPtr printFace, PlaneEq planeEq) {
    ksPartPtr part(document3d->GetPart(pTop_Part));

    ksMeasurerPtr measurer(part->GetMeasurer());

    ksBodyPtr body = part->GetMainBody();
    ksFaceCollectionPtr faces = body->FaceCollection();
    int facesCount = faces->GetCount();
    std::set<ksFaceDefinitionPtr> holes;
    for (int faceIndex = 0; faceIndex < facesCount; faceIndex++) {
        ksFaceDefinitionPtr face = faces->GetByIndex(faceIndex);
        if (face->IsPlanar()) {
            ksLoopCollectionPtr loops(face->LoopCollection());
            for (int loopIndex = 0; loopIndex < loops->GetCount(); loopIndex++) {
                ksLoopPtr innerLoop(loops->GetByIndex(loopIndex));
                if (!(innerLoop->IsOuter())) {
                    ksEdgeCollectionPtr edges(innerLoop->EdgeCollection());
                    if (edges->GetCount() == 1) {
                        ksEdgeDefinitionPtr edge(edges->GetByIndex(0));
                        if ((edge->IsCircle() || edge->IsEllipse()) && !checkAngle(document3d, edge)) {
                            ksFaceDefinitionPtr otherFace = nullptr;
                            if (edge->GetAdjacentFace(true) != face) {
                                otherFace = edge->GetAdjacentFace(true);
                            } else {
                                otherFace = edge->GetAdjacentFace(false);
                            }
                            if (otherFace->IsCylinder()) {
                                ksEdgeCollectionPtr otherFaceEdges(otherFace->EdgeCollection());
                                if (otherFaceEdges->GetCount() == 2) {
                                    ksEdgeDefinitionPtr otherEdge = otherFaceEdges->GetByIndex(0);
                                    if (otherEdge == edge) {
                                        otherEdge = otherFaceEdges->GetByIndex(1);
                                    }
                                    if ((otherEdge->IsCircle() || otherEdge->IsEllipse()) && !checkAngle(document3d, otherEdge)) {
                                        holes.insert(otherFace);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return holes;
}

std::vector<ILineSegmentPtr> createTriangle(IDrawingContainerPtr drawingContainer,
    double p_x_1, double p_x_2, double p_x_3, double p_x_c,
    double p_y_1, double p_y_2, double p_y_3, double p_y_c,
    ICirclePtr circle, ILinePtr line, ISymbols2DContainerPtr symbols2dContainer,
    double overhangThreshold
    ) {
    ILineSegmentPtr line1(drawingContainer->LineSegments->Add());
    ILineSegmentPtr line2(drawingContainer->LineSegments->Add());
    ILineSegmentPtr line3(drawingContainer->LineSegments->Add());
    line1->X1 = p_x_1;
    line1->X2 = p_x_2;
    line2->X1 = p_x_2;
    line2->X2 = p_x_3;
    line3->X1 = p_x_3;
    line3->X2 = p_x_1;
    line1->Y1 = p_y_1;
    line1->Y2 = p_y_2;
    line2->Y1 = p_y_2;
    line2->Y2 = p_y_3;
    line3->Y1 = p_y_3;
    line3->Y2 = p_y_1;
    line1->Update();
    line2->Update();
    line3->Update();
    ConstraintsCreator c1(line1);
    ConstraintsCreator c3(line3);
    ConstraintsCreator c2(line2);
    ConstraintsCreator c_curve1(line1);
    ConstraintsCreator c_curve2(line3);
    ConstraintsCreator c_curve3(line1);

    ConstraintsCreator c_eq(line1);

    c1.mergePoints(0, line3, 1);
    c3.mergePoints(0, line2, 1);
    c2.mergePoints(0, line1, 1);

    c_eq.equalLength(line3);

    c_curve1.pointOnCurve(1, circle);
    c_curve2.pointOnCurve(0, circle);
    c_curve3.pointOnCurve(0, line);
    IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);

    IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
    angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
    angleDim->BaseObject1 = IDrawingObjectPtr(line1);
    angleDim->BaseObject2 = IDrawingObjectPtr(line3);
    angleDim->Radius = 0;
    angleDim->X3 = (line1->X1 + line3->X2) / 2;
    angleDim->Y3 = (line1->Y1 + line3->Y2) / 2;
    angleDim->Update();

    std::ostringstream oss;
    oss << overhangThreshold;
    CComBSTR temp(oss.str().c_str());
    _bstr_t expression = temp.Detach();

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

    std::vector <ILineSegmentPtr> lines = { line1, line2, line3 };
    return lines;
}


void createMaxAngleDim(IDrawingContainerPtr drawingContainer, ISymbols2DContainerPtr symbols2dContainer, ILineSegmentPtr line1, ILineSegmentPtr line2, _bstr_t value) {
    //IPointPtr  p0(drawingContainer->Points->Add());
    IAngleDimensionsPtr angleDimensions(symbols2dContainer->AngleDimensions);
    IAngleDimensionPtr angleDim(angleDimensions->Add(DrawingObjectTypeEnum::ksDrADimension));
    angleDim->DimensionType = ksAngleDimTypeEnum::ksADMinAngle;
    angleDim->BaseObject1 = IDrawingObjectPtr(line1);
    angleDim->BaseObject2 = IDrawingObjectPtr(line2);
    double x = (((line1->X1 + line1->X2) / 2.0) + ((line2->X1 + line2->X2) / 2.0))/ 2.0;
    double y = (((line1->Y1 + line1->Y2) / 2.0) + ((line2->Y1 + line2->Y2) / 2.0))/ 2.0;
    angleDim->X3 = x;
    angleDim->Y3 = y;
    //p0->X = x;
    //p0->Y = y;
    //p0->Update();
    angleDim->Radius = 0;
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
        constraint->Expression = value;//"115.5288";
        constraint->Create();
    }
}

void axisConstr(IDrawingContainerPtr drawingContainer, ISymbols2DContainerPtr symbols2dContainer, std::vector<ILineSegmentPtr> lines1, std::vector<ILineSegmentPtr> lines2) {
    ILineSegmentPtr line1_1 = lines1[0];
    ILineSegmentPtr line1_2 = lines1[1];
    ILineSegmentPtr line1_3 = lines1[2];

    ILineSegmentPtr line2_1 = lines2[0];
    ILineSegmentPtr line2_2 = lines2[1];
    ILineSegmentPtr line2_3 = lines2[2];

    ILineSegmentPtr ax1(drawingContainer->LineSegments->Add());
    ax1->X1 = line2_2->X1;
    ax1->Y1 = line2_2->Y1;
    ax1->X2 = line1_2->X1;
    ax1->Y2 = line1_2->Y1;

    ILineSegmentPtr ax2(drawingContainer->LineSegments->Add());
    ax2->X1 = line2_2->X2; 
    ax2->Y1 = line2_2->Y2;
    ax2->X2 = line1_2->X2;
    ax2->Y2 = line1_2->Y2;
    ax2->Style = ksCurveStyleEnum::ksCSThin;
    ax1->Style = ksCurveStyleEnum::ksCSThin;
    ax1->Update();
    ax2->Update();

    ConstraintsCreator c1_2_0(line1_2);
    c1_2_0.mergePoints(0, ax1, 0);
    ConstraintsCreator c2_2_0(line2_2);
    c2_2_0.mergePoints(0, ax1, 1);
    ConstraintsCreator c1_2_1(line1_2);
    c1_2_1.mergePoints(1, ax2, 0);
    ConstraintsCreator c2_2_1(line2_2);
    c2_2_1.mergePoints(1, ax2, 1);
    ConstraintsCreator c_eq(line1_2);
    c_eq.equalLength(line2_2);
    CComBSTR temp("64.4712");
    _bstr_t expression = temp.Detach();
    createMaxAngleDim(drawingContainer, symbols2dContainer, ax1, line1_1, expression);
}

void optimizeCircleHorizontalHoles(KompasObjectPtr kompas, double slotAngle, ksFaceDefinitionPtr printFace, PlaneEq printPlaneEq) {
    
    IApplicationPtr api7 = kompas->ksGetApplication7();
    IKompasDocument3DPtr document3d(api7->GetActiveDocument());
    IPart7Ptr topPart(document3d->GetTopPart());
    ksPartPtr part = kompas->TransferInterface(topPart, 1, 0);

    ksEntityPtr mainMacroElementEntity(part->NewEntity(o3d_MacroObject));
    ksMacro3DDefinitionPtr mainMacroElement(mainMacroElementEntity->GetDefinition());
    mainMacroElementEntity->name = "Оптимизация горизонтальных отверстий";
    mainMacroElement->StaffVisible = true;
    mainMacroElementEntity->Create();

    ksDocument3DPtr doc3d = kompas->ActiveDocument3D();
    std::set<ksFaceDefinitionPtr> holes = getHorizontalCircleHoles(doc3d, printFace, printPlaneEq);
    std::vector<ksEntityPtr> toRemove;

    std::cout << "holes number:" << holes.size() << "\n";
    ksMeasurerPtr measurer(part->GetMeasurer());
    IKompasDocument3DPtr doc(api7->ActiveDocument);
    IPart7Ptr part7(doc->TopPart);
    IModelContainerPtr modelcontainer(part7);
    IPoints3DPtr points3D(modelcontainer->Points3D);
    for (std::set<ksFaceDefinitionPtr>::iterator iter = holes.begin(); iter != holes.end(); iter++) {
        bool removeItPls = false;
        ksEntityPtr macroElementEntity(part->NewEntity(o3d_MacroObject));
        ksMacro3DDefinitionPtr macroElement(macroElementEntity->GetDefinition());
        macroElementEntity->name = "Объекты построенния";
        macroElement->StaffVisible = true;
        macroElementEntity->Create();

        ksFaceDefinitionPtr face = *iter;
        ksFaceDefinitionPtr face1(ksFaceCollectionPtr(face->ConnectedFaceCollection())->GetByIndex(0));
        ksFaceDefinitionPtr face2(ksFaceCollectionPtr(face->ConnectedFaceCollection())->GetByIndex(1));

        double r, h;
        face->GetCylinderParam(&h, &r);
        ksEntityPtr axisEntity(part->NewEntity(o3d_axisConeFace));
        ksAxisConefaceDefinitionPtr axis(axisEntity->GetDefinition());
        axis->SetFace(face);
        ksVertexDefinitionPtr vertex(ksEdgeDefinitionPtr(ksEdgeCollectionPtr(face->EdgeCollection())->First())->GetVertex(true));
        axisEntity->hidden = true;
        axisEntity->Create();
        macroElement->Add(axisEntity);
        bool check = true;
        measurer->SetObject1(printFace->GetEntity());
        measurer->SetObject2(axisEntity);
        measurer->Calc();

        double angle = abs(measurer->angle);
        if (angle < EPS_ANGLE) {
            ksEntityPtr mainPlaneEntity(part->NewEntity(o3d_planePerpendicular));
            ksPlanePerpendicularDefinitionPtr mainPlane(mainPlaneEntity->GetDefinition());
            mainPlane->SetPoint(vertex);
            mainPlane->SetEdge(axis);
            mainPlaneEntity->hidden = true;
            mainPlaneEntity->Create();
            macroElement->Add(mainPlaneEntity);
            ksEntityPtr secondPlaneEntity(part->NewEntity(o3d_planeLineToPlane));
            ksPlaneLineToPlaneDefinitionPtr secondPlane(secondPlaneEntity->GetDefinition());
            secondPlane->SetEdge(axis);
            secondPlane->SetPlane(printFace);
            secondPlane->parallel = false;
            secondPlaneEntity->hidden = true;
            secondPlaneEntity->Create();
            macroElement->Add(secondPlane);

            ksEntityPtr axis2 = makeAxis(part, mainPlaneEntity, secondPlaneEntity);
            Sketch sketch = createSketch(kompas, part, mainPlaneEntity);
            sketch.definition->AddProjectionOf(face);
            sketch.definition->AddProjectionOf(axis2);
            macroElement->Add(axis2);
            

            IViewsAndLayersManagerPtr viewsAndLayersManager(sketch.document2d_api7->ViewsAndLayersManager);
            IViewsPtr views(viewsAndLayersManager->Views);
            IViewPtr view(views->ActiveView);
            IDrawingContainerPtr drawingContainer(view);
            ICirclesPtr circles(drawingContainer->Circles);
            ILinesPtr lines(drawingContainer->Lines);
            if (lines->Count > 0 && circles->Count > 0) {
                ILinePtr line(lines->GetLine(0));
                ICirclePtr circle(circles->GetCircle(0));
                for (int i = 0; i < circles->Count; i++) {
                    ICirclePtr currCircle(circles->GetCircle(i));
                    currCircle->Style = ksCurveStyleEnum::ksCSThin;
                    currCircle->Update();
                }
                double x = line->X2 - line->X1, y = line->Y2 - line->Y1;
                double length = sqrt((x * x) + (y * y));
                double x_vect = x / length, y_vect = y / length; //единичный вектор
                double r = circle->Radius;
                double x0 = circle->Xc, y0 = circle->Yc;

                double sin_alpha = 1.0 / 3.0;
                double cos_alpha = (2.0 * sqrt(2.0)) / 3.0;

                double x_diag_vector_1 = (x_vect * cos_alpha) - (y_vect * sin_alpha),
                    y_diag_vector_1 = (x_vect * sin_alpha) + (y_vect * cos_alpha); // единичный вектор повернутый на 45 град

                double x_diag_vector_2 = (x_vect * cos_alpha) + (y_vect * sin_alpha),
                    y_diag_vector_2 = -(x_vect * sin_alpha) + (y_vect * cos_alpha); // единичный вектор повёрнутый на -45 град
                
                double tg_max_angle = tan(((slotAngle/2) * M_PI) / 180);
                double ext_dist = r / (3.0 * tg_max_angle);
                double p1_x = x0 + ((r + ext_dist) * x_vect), p1_y = y0 + ((r + ext_dist) * y_vect);
                double p1_x_1 = x0 + (r * x_diag_vector_1), p1_y_1 = y0 + (r * y_diag_vector_1);
                double p1_x_2 = x0 + (r * x_diag_vector_2), p1_y_2 = y0 + (r * y_diag_vector_2);

                double p2_x = x0 - ((r + ext_dist) * x_vect), p2_y = y0 - ((r + ext_dist) * y_vect);
                double p2_x_1 = x0 - (r * x_diag_vector_1), p2_y_1 = y0 - (r * y_diag_vector_1);
                double p2_x_2 = x0 - (r * x_diag_vector_2), p2_y_2 = y0 - (r * y_diag_vector_2);
                ISymbols2DContainerPtr symbols2dContainer(view);

                std::vector<ILineSegmentPtr> lines1 = createTriangle(drawingContainer, p1_x, p1_x_1, p1_x_2, x0, p1_y, p1_y_1, p1_y_2, y0, circle, line, symbols2dContainer, slotAngle);
                std::vector<ILineSegmentPtr> lines2 = createTriangle(drawingContainer, p2_x, p2_x_1, p2_x_2, x0, p2_y, p2_y_1, p2_y_2, y0, circle, line, symbols2dContainer, slotAngle);
                axisConstr(drawingContainer, symbols2dContainer, lines1, lines2);
                
            } else {
                removeItPls = true;
                std::cout << "error\n";
            }
            sketch.definition->EndEdit();
            ksEntityPtr cutEntity = createCute(part, sketch, face1, face2);
            macroElement->Add(cutEntity);
            macroElement->Add(sketch.entity);

        } else {
            removeItPls = true;
        }
        if (!removeItPls) {
            mainMacroElement->Add(macroElementEntity);
        } else {
            toRemove.push_back(axisEntity);
            toRemove.push_back(macroElementEntity);
        }
    }
    for (std::vector<ksEntityPtr>::iterator iter = toRemove.begin(); iter != toRemove.end(); iter++) {
        doc3d->DeleteObject(*iter);
    }
    mainMacroElementEntity->Update();
    doc3d->RebuildDocument();
}