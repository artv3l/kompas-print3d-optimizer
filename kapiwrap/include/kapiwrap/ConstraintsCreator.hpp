#pragma once

#include <comutil.h>

#include <KsAPI.h>

class ConstraintsCreator {
public:
    ConstraintsCreator(kapi::IDrawingObjectPtr drawingObject);
    ConstraintsCreator(ksapi::IDrawingObjectPtr drawingObject);

    bool pointOnCurve(long index, kapi::IDrawingObjectPtr partner);                             // 2
    bool pointOnCurve(long index, ksapi::IDrawingObjectPtr partner);                            // 2
    bool horizontal();                                                                          // 3
    bool parallel(kapi::IDrawingObjectPtr partner);                                             // 5
    bool equalLength(kapi::IDrawingObjectPtr partner);                                          // 7
    bool equalRadius(kapi::IDrawingObjectPtr partner);                                          // 8
    bool equalRadius(ksapi::IDrawingObjectPtr partner);                                         // 8
    bool horizontalAlignPoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex); // 9
    bool mergePoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex);           // 11
    bool mergePoints(long index, ksapi::IDrawingObjectPtr partner, long partnerIndex);          // 11
    bool dimWithVariable(_bstr_t expression);                                                   // 13
    bool dimWithVariable(std::wstring_view expression);                                         // 13
    bool fixedDim();                                                                            // 14
    bool tangentTwoCurves(kapi::IDrawingObjectPtr partner);                                     // 15
    bool tangentTwoCurves(ksapi::IDrawingObjectPtr partner);                                    // 15
    
private:
    ksapi::IDrawingObjectPtr m_drawingObject;
    kapi::IDrawingObject1Ptr m_drawingObject1;

};
