#ifndef CONSTRAINTS_CREATOR_HPP
#define CONSTRAINTS_CREATOR_HPP

#include <comutil.h>

class ConstraintsCreator {
public:
    ConstraintsCreator(kapi::IDrawingObjectPtr drawingObject);

    bool pointOnCurve(long index, kapi::IDrawingObjectPtr partner);                             // 2
    bool horizontal();                                                                          // 3
    bool parallel(kapi::IDrawingObjectPtr partner);                                             // 5
    bool equalLength(kapi::IDrawingObjectPtr partner);                                          // 7
    bool equalRadius(kapi::IDrawingObjectPtr partner);                                          // 8
    bool horizontalAlignPoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex); // 9
    bool mergePoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex);           // 11
    bool dimWithVariable(_bstr_t expression);                                                   // 13
    bool fixedDim();                                                                            // 14
    bool tangentTwoCurves(kapi::IDrawingObjectPtr partner);                                     // 15
    
private:
    kapi::IDrawingObjectPtr m_drawingObject;
    kapi::IDrawingObject1Ptr m_drawingObject1;

};

#endif /* CONSTRAINTS_CREATOR_HPP */
