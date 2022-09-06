#include "stdafx.h"
#include "optimizeRounding.hpp"

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include "selectPlane.hpp"

bool checkEdge(ksMeasurerPtr measurer, ksEdgeDefinitionPtr edge, double radius) {
    if (edge) {
        ksFaceDefinitionPtr face1(edge->GetAdjacentFace(true));
        ksFaceDefinitionPtr face2(edge->GetAdjacentFace(false));
        if (face1 && face2) {
            /*
            double area1 = face1->GetArea(0x1);
            double area2 = face2->GetArea(0x1);
            double lenOfEdge = edge->GetLength(0x1);
            double maxRadius = DBL_MAX;
            ksVertexDefinitionPtr v1(edge->GetVertex(true));
            ksVertexDefinitionPtr v2(edge->GetVertex(false));
            if (v1 && v2) {
                double x1, x2, y1, y2, z1, z2;
                v1->GetPoint(&x1, &y1, &z1);
                v2->GetPoint(&x2, &y2, &z2);
                double x = x2 - x1, y = y2 - y1, z = z2 - z1; //координаты вектора ребра

                ksFaceDefinitionPtr face1(edge->GetAdjacentFace(true));
                ksFaceDefinitionPtr face2(edge->GetAdjacentFace(false));
                ksFaceCollectionPtr faces1(face1->ConnectedFaceCollection());
                ksFaceCollectionPtr faces2(face1->ConnectedFaceCollection());
                for (int i = 0; i < faces1->GetCount(); i++) {
                    ksFaceDefinitionPtr currFace = faces1->GetByIndex(i);
                    if (currFace->IsPlanar() && currFace != face2 && currFace != face1) {
                        measurer->SetObject1(currFace);
                        measurer->SetObject2(edge);
                        measurer->Calc();
                        if (measurer->Getangle() == 0) {
                            if (measurer->distance < maxRadius) {
                                maxRadius = measurer->distance;
                            }
                        }

                    }
                }
                for (int i = 0; i < faces2->GetCount(); i++) {
                    ksFaceDefinitionPtr currFace = faces2->GetByIndex(i);
                    if (currFace->IsPlanar() && currFace != face1 && currFace != face2) {
                        measurer->SetObject1(currFace);
                        measurer->SetObject2(edge);
                        measurer->Calc();
                        if (measurer->Getangle() == 0) {
                            if (measurer->distance < maxRadius) {
                                maxRadius = measurer->distance;
                            }
                        }

                    }
                }
                std::cout << "maxRadius:" << maxRadius <<"\n";
                */
            return edge->IsStraight() && face1->IsPlanar() && face2->IsPlanar();
            //}
        }
    }
    return false;

}


void optimizeByRounding(KompasObjectPtr kompas, ksFaceDefinitionPtr face, PlaneEq planeEq, double radius, double angle) {
    double cos_angle = sin(angle * M_PI / 180.0); //да, тут смежные углы
    IApplicationPtr api7 = kompas->ksGetApplication7();
    IKompasDocument3DPtr document3d(api7->GetActiveDocument());
    IPart7Ptr topPart(document3d->GetTopPart());
    ksPartPtr part = kompas->TransferInterface(topPart, 1, 0);
    ksFeaturePtr feature(part->GetFeature());
    ksEntityCollectionPtr entityCollection(feature->EntityCollection(o3d_edge));
    ksMeasurerPtr measurer(part->GetMeasurer());
    ksEntityPtr macroElementEntity(part->NewEntity(o3d_MacroObject));
    ksMacro3DDefinitionPtr macroElement(macroElementEntity->GetDefinition());
    macroElementEntity->name = "Оптимизирующие скругления для выпирающих углов";
    macroElement->StaffVisible = true;
    macroElementEntity->Create();
    for (int i = 0; i < entityCollection->GetCount(); i++) {
        ksEntityPtr entity(entityCollection->GetByIndex(i));
        ksEdgeDefinitionPtr edge(entity->GetDefinition());
        if (edge && checkEdge(measurer, edge, radius)) {
            bool isVerical = planeEq.isVertical(edge, cos_angle);
            if (isVerical) {
                std::cout << "найдено вертикальное ребро" << "\n";
                ksEntityPtr filletEntity(part->NewEntity(o3d_fillet));
                ksFilletDefinitionPtr fillet(filletEntity->GetDefinition());
                ksEntityCollectionPtr array(fillet->array());
                fillet->radius = radius;
                array->Add(entity);
                filletEntity->Create();
                if (filletEntity->IsCreated()) {
                    macroElement->Add(filletEntity);
                }
            } else {
                std::cout << "найдено невертикальное ребро" << "\n";
            }
        }
    }
    macroElementEntity->Update();
}
