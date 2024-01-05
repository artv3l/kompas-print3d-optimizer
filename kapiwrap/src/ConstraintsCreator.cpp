#include "ConstraintsCreator.hpp"

#include <comutil.h>

ConstraintsCreator::ConstraintsCreator(IDrawingObjectPtr drawingObject):
    m_drawingObject(drawingObject),
    m_drawingObject1(drawingObject) {
}

bool ConstraintsCreator::pointOnCurve(long index, IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCPointOnCurve;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontal() {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCHorizontal;
    return constraint->Create();
}

bool ConstraintsCreator::parallel(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCParallel;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::equalLength(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCEqualLength;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::equalRadius(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCEqualRadius;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontalAlignPoints(long index, IDrawingObjectPtr partner, long partnerIndex) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCHAlignPoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::mergePoints(long index, IDrawingObjectPtr partner, long partnerIndex) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCMergePoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::dimWithVariable(_bstr_t expression) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCDimWithVariable;
    constraint->Expression = expression;
    return constraint->Create();
}

bool ConstraintsCreator::fixedDim() {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCFixedDim;
    return constraint->Create();
}

bool ConstraintsCreator::tangentTwoCurves(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCTangentTwoCurves;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}
