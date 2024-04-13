#include "ConstraintsCreator.hpp"

#include <comutil.h>

ConstraintsCreator::ConstraintsCreator(kapi::IDrawingObjectPtr drawingObject):
    m_drawingObject(drawingObject),
    m_drawingObject1(drawingObject) {
}

bool ConstraintsCreator::pointOnCurve(long index, kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCPointOnCurve;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontal() {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCHorizontal;
    return constraint->Create();
}

bool ConstraintsCreator::parallel(kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCParallel;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::equalLength(kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCEqualLength;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::equalRadius(kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCEqualRadius;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontalAlignPoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCHAlignPoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::mergePoints(long index, kapi::IDrawingObjectPtr partner, long partnerIndex) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCMergePoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::dimWithVariable(_bstr_t expression) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCDimWithVariable;
    constraint->Expression = expression;
    return constraint->Create();
}

bool ConstraintsCreator::fixedDim() {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCFixedDim;
    return constraint->Create();
}

bool ConstraintsCreator::tangentTwoCurves(kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCTangentTwoCurves;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}
