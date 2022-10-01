#include "stdafx.h"
#include "ConstraintsCreator.hpp"

ConstraintsCreator::ConstraintsCreator(IDrawingObjectPtr drawingObject):
    drawingObject_(drawingObject),
    drawingObject1_(drawingObject) {
}

bool ConstraintsCreator::pointOnCurve(long index, IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCPointOnCurve;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontal() {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCHorizontal;
    return constraint->Create();
}

bool ConstraintsCreator::parallel(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCParallel;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::horizontalAlignPoints(long index, IDrawingObjectPtr partner, long partnerIndex) {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCHAlignPoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::mergePoints(long index, IDrawingObjectPtr partner, long partnerIndex) {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCMergePoints;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    constraint->PartnerIndex = partnerIndex;
    return constraint->Create();
}

bool ConstraintsCreator::tangentTwoCurves(IDrawingObjectPtr partner) {
    IParametriticConstraintPtr constraint(drawingObject1_->NewConstraint());
    constraint->ConstraintType = ksConstraintTypeEnum::ksCTangentTwoCurves;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}
