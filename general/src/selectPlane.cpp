#include "stdafx.h"
#include "selectPlane.hpp"

#include <iostream>

const double EPS_PLANE_EQ = 0.001;

void checkPlane(KompasObjectPtr kompas, double a, double b, double c, double d, int* s1, int* s2) {
    *s1 = 0;
    *s2 = 0;
    IApplicationPtr api7 = kompas->ksGetApplication7();
    IKompasDocument3DPtr document3d(api7->GetActiveDocument());
    IPart7Ptr topPart(document3d->GetTopPart());
    ksPartPtr part = kompas->TransferInterface(topPart, 1, 0);
    ksFeaturePtr feature(part->GetFeature());
    ksEntityCollectionPtr entityCollection(feature->EntityCollection(o3d_vertex));
    entityCollection->GetCount();
    for (int i = 0; i < entityCollection->GetCount(); i++) {
        ksEntityPtr entity(entityCollection->GetByIndex(i));
        ksVertexDefinitionPtr vertex(entity->GetDefinition());
        double x, y, z;
        if (vertex) {
            vertex->GetPoint(&x, &y, &z);
            if ((x * a) + (y * b) + (z * c) + d > PLANE_BORDER_EPS) {
                (*s1)++;
            } else if ((x * a) + (y * b) + (z * c) + d < -PLANE_BORDER_EPS) {
                (*s2)++;
            }
        }
    }
}

PlaneEq::PlaneEq(ksFaceDefinitionPtr face) {
    double x0, y0, z0;
    ksSurfacePtr surface(face->GetSurface());
    surface->GetPoint(surface->GetParamUMax(), surface->GetParamVMax(), &x0, &y0, &z0);
    double a, b, c, d;
    surface->GetNormal(surface->GetParamUMax(), surface->GetParamVMax(), &a, &b, &c);
    d = -((a * x0) + (b * y0) + (c * z0));
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
}

bool PlaneEq::equals(PlaneEq other) {
    double k;
    if (abs(other.a) > EPS_PLANE_EQ) {
        k = a / other.a;
    } else if (abs(other.b) > EPS_PLANE_EQ) {
        k = b / other.b;
    } else if (abs(other.c) > EPS_PLANE_EQ) {
        k = c / other.c;
    } else if (abs(other.d) > EPS_PLANE_EQ) {
        k = d / other.d;
    } else {
        return false;
    }
    double a2 = other.a * k;
    double b2 = other.b * k;
    double c2 = other.c * k;
    double d2 = other.d * k;

    return (abs(a - a2) < EPS_PLANE_EQ) && (abs(b - b2) < EPS_PLANE_EQ) && (abs(c - c2) < EPS_PLANE_EQ) && (abs(d - d2) < EPS_PLANE_EQ);
}

PlaneEq::PlaneEq() {
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->d = 0;
}

bool PlaneEq::isVertical(ksEdgeDefinitionPtr edge, double cos_angle) {
    if (edge) {
        double x1, x2, y1, y2, z1, z2, x, y, z;
        ksVertexDefinitionPtr vertex1(edge->GetVertex(true));
        ksVertexDefinitionPtr vertex2(edge->GetVertex(false));
        if (vertex1 && vertex2) {
            vertex1->GetPoint(&x1, &y1, &z1);
            vertex2->GetPoint(&x2, &y2, &z2);
            x = x2 - x1;
            y = y2 - y1;
            z = z2 - z1;
            std::cout << "edge vector: " << " x:" << x << " y:" << y << " z:" << z << "\n";
            std::cout << "n vector: " << " x" << a << " y:" << b << " z:" << c << "\n";

            double n_mul_vect = (a * x) + (b * y) + (c * z);
            double n_abs = sqrt((a * a) + (b * b) + (c * c));
            double vect_abs = sqrt((x * x) + (y * y) + (z * z));
            if ((n_abs > 0.001) && (vect_abs > 0.001)) {
                double cos_n_vect = n_mul_vect / (n_abs * vect_abs);
                std::cout << "cos: " << cos_n_vect << "\n";
                std::cout << "limit cos: " << cos_angle << "\n";

                return abs(cos_n_vect) > cos_angle;
            }
        }
    }
    return false;
}

/*
    Возвращает выбранную пользователем грань, если она удовлетворяет всем требованиям.
    A также через аргумент *planeEq, передаёт коэф. уравнения плоскости.
*/
PrintPlane getSelectedPlane(KompasObjectPtr kompas) {
    PlaneEq planeEq;
    ksDocument3DPtr doc3d = kompas->ActiveDocument3D();
    ksSelectionMngPtr selectionMng(doc3d->GetSelectionMng());
    std::cout << "Количество выбранных элементов:" << selectionMng->GetCount() << "\n";
    if (selectionMng->GetCount() == 1 && selectionMng->GetObjectType(0) == 105) {
        ksEntityPtr element(selectionMng->GetObjectByIndex(0));
        if (element->type == 6) {
            std::cout << "Была выбрана грань" << "\n";
            ksFaceDefinitionPtr face(element->GetDefinition());
            if (face->IsPlanar()) {
                std::cout << "Грань является плоской" << "\n";

                ksSurfacePtr surface(face->GetSurface());
                double x0, y0, z0;
                surface->GetPoint(surface->GetParamUMax(), surface->GetParamVMax(), &x0, &y0, &z0);
                double a, b, c, d;

                surface->GetNormal(surface->GetParamUMax(), surface->GetParamVMax(), &a, &b, &c);

                d = -((a * x0) + (b * y0) + (c * z0));

                std::cout << "Вектор нормали: X=" << a << " Y="
                    << b << " Z=" << c << "\n";

                std::cout << "Координаты точки отсчёта: X=" << x0 << " Y="
                    << y0 << " Z=" << z0 << "\n";

                std::cout << "Полученые коэф. уравнения плоск: A=" << a << " B="
                    << b << " C=" << c << " D=" << d << "\n";

                int s1 = 0, s2 = 0;

                std::cout << "Идёт проверка на перечения..." << "\n";

                checkPlane(kompas, a, b, c, d, &s1, &s2);
                std::cout << "Сторона 1:" << s1 << " Сторона 2:" << s2 << " \n";

                if (s1 == 0 && s2 > 0) {
                    std::cout << "Уравнение плоскости отражено" << "\n";
                    a = -a;
                    b = -b;
                    c = -c;
                    d = -d;
                    int buff = s2;
                    s2 = s1;
                    s1 = buff;
                }


                if (!((s1 > 0) && (s2 == 0))) {
                    kompas->ksMessage("Плоскость печати пересекает деталь!");
                } else {
                    kompas->ksMessage("Плоскость печати успешно выбрана!");
                    planeEq.a = a;
                    planeEq.b = b;
                    planeEq.c = c;
                    planeEq.d = d;
                    return PrintPlane{face, planeEq};
                }

            } else {
                kompas->ksMessage("Выбранная грань должна быть плоской!");
            }
        } else {
            kompas->ksMessage("Выбранный элемент не является гранью!");
        }
    } else if (selectionMng->GetCount() > 0) {
        kompas->ksMessage("Должен был быть выбран только один элемент в виде плоской грани!");
    } else {
        kompas->ksMessage("Плоскость печати не выбрана!");
    }
    return PrintPlane{nullptr, planeEq};
}
