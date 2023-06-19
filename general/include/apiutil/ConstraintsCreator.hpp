#ifndef CONSTRAINTS_CREATOR_HPP
#define CONSTRAINTS_CREATOR_HPP

class ConstraintsCreator {
public:
    ConstraintsCreator(IDrawingObjectPtr drawingObject);

    bool pointOnCurve(long index, IDrawingObjectPtr partner);                             // 2
    bool horizontal();                                                                    // 3
    bool parallel(IDrawingObjectPtr partner);                                             // 5
    bool equalLength(IDrawingObjectPtr partner);                                          // 7
    bool equalRadius(IDrawingObjectPtr partner);                                          // 8
    bool horizontalAlignPoints(long index, IDrawingObjectPtr partner, long partnerIndex); // 9
    bool mergePoints(long index, IDrawingObjectPtr partner, long partnerIndex);           // 11
    bool tangentTwoCurves(IDrawingObjectPtr partner);                                     // 15
    
private:
    IDrawingObjectPtr drawingObject_;
    IDrawingObject1Ptr drawingObject1_;

};

#endif /* CONSTRAINTS_CREATOR_HPP */
