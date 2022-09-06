#include "stdafx.h"
#include "optimizeCircleHorizontalHoles.hpp"

#include <iostream>
#include <set>

#define _USE_MATH_DEFINES
#include <math.h>

#include "utils.hpp"
#include "concaveAngle.hpp"
#include "selectPlane.hpp"

#define EPS_ANGLE 0.001
#define EPS_DISTANCE 0.00001

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
    if (checkPlaneEntities(part, sketch.entity, depthFace1->GetEntity())) { //выт€гиваем до depthFace2
        cut->directionType = (short)Direction_Type::dtNormal;
        cut->SetSideParam(true, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(true, depthFace2->GetEntity());
        cut->SetSketch(sketch.entity);
        cutEntity->Create();
    } else if (checkPlaneEntities(part, sketch.entity, depthFace2->GetEntity())) { //выт€гиваем до depthFace1
        cut->directionType = (short)Direction_Type::dtNormal;
        cut->SetSideParam(true, (short)End_Type::etUpToSurfaceTo, 0, 0, false);
        cut->SetDepthObject(true, depthFace1->GetEntity());
        cut->SetSketch(sketch.entity);
        cutEntity->Create();
    } else { //выт€гиваем до depthFace1 и до depthFace2
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
                        if ((edge->IsCircle() || edge->IsEllipse()) && !isConcaveAngle(document3d, edge)) {
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
                                    if ((otherEdge->IsCircle() || otherEdge->IsEllipse()) && !isConcaveAngle(document3d, otherEdge)) {
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

void optimizeCircleHorizontalHoles(KompasObjectPtr kompas, double maxAngle, ksFaceDefinitionPtr printFace, PlaneEq printPlaneEq) {
    double tg_max_angle = tan((maxAngle * M_PI) / 180);
    IApplicationPtr api7 = kompas->ksGetApplication7();
    IKompasDocument3DPtr document3d(api7->GetActiveDocument());
    IPart7Ptr topPart(document3d->GetTopPart());
    ksPartPtr part = kompas->TransferInterface(topPart, 1, 0);

    ksEntityPtr mainMacroElementEntity(part->NewEntity(o3d_MacroObject));
    ksMacro3DDefinitionPtr mainMacroElement(mainMacroElementEntity->GetDefinition());
    mainMacroElementEntity->name = "ќптимизаци€ горизонтальных отверстий";
    mainMacroElement->StaffVisible = true;
    mainMacroElementEntity->Create();

    ksDocument3DPtr doc3d = kompas->ActiveDocument3D();
    std::set<ksFaceDefinitionPtr> holes = getHorizontalCircleHoles(doc3d, printFace, printPlaneEq);
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
        macroElementEntity->name = "ќбъекты построенни€";
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
        measurer->SetObject1(printFace->GetEntity());
        measurer->SetObject2(axisEntity);
        measurer->Calc();
        //std::cout << "angle:" << abs(measurer->angle) << "\n";
        if (abs(measurer->angle) < EPS_ANGLE) {
            //std::cout << "OK!\n";
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
                    y_diag_vector_2 = -(x_vect * sin_alpha) + (y_vect * cos_alpha); // единичный вектор повЄрнутый на -45 град

                double ext_dist = r / (3.0 * tg_max_angle);
                double p1_x = x0 + ((r + ext_dist) * x_vect), p1_y = y0 + ((r + ext_dist) * y_vect);
                double p1_x_1 = x0 + (r * x_diag_vector_1), p1_y_1 = y0 + (r * y_diag_vector_1);
                double p1_x_2 = x0 + (r * x_diag_vector_2), p1_y_2 = y0 + (r * y_diag_vector_2);

                double p2_x = x0 - ((r + ext_dist) * x_vect), p2_y = y0 - ((r + ext_dist) * y_vect);
                double p2_x_1 = x0 - (r * x_diag_vector_1), p2_y_1 = y0 - (r * y_diag_vector_1);
                double p2_x_2 = x0 - (r * x_diag_vector_2), p2_y_2 = y0 - (r * y_diag_vector_2);


                IPolyLine2DPtr polyLine1(drawingContainer->PolyLines2D->Add());
                polyLine1->AddPoint(0, p1_x, p1_y);
                polyLine1->AddPoint(1, p1_x_1, p1_y_1);
                polyLine1->AddPoint(2, p1_x_2, p1_y_2);
                polyLine1->AddPoint(3, p1_x, p1_y);
                polyLine1->Update();

                IPolyLine2DPtr polyLine2(drawingContainer->PolyLines2D->Add());
                polyLine2->AddPoint(0, p2_x, p2_y);
                polyLine2->AddPoint(1, p2_x_1, p2_y_1);
                polyLine2->AddPoint(2, p2_x_2, p2_y_2);
                polyLine2->AddPoint(3, p2_x, p2_y);
                polyLine2->Update();
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
        macroElementEntity->Update();
        if (!removeItPls) {
            mainMacroElement->Add(macroElementEntity);
        } else {
            doc3d->DeleteObject(macroElementEntity);
            //doc3d->RebuildDocument();

        }
    }
    mainMacroElementEntity->Update();
}
