#include "kapiwrap/ConstraintsCreator.hpp"

#include <comutil.h>

ConstraintsCreator::ConstraintsCreator(kapi::IDrawingObjectPtr drawingObject):
    m_drawingObject1(drawingObject)
{
}

ConstraintsCreator::ConstraintsCreator(ksapi::IDrawingObjectPtr drawingObject) :
    m_drawingObject(drawingObject)
{
}

bool ConstraintsCreator::pointOnCurve(long index, kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCPointOnCurve;
    constraint->Index = index;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::pointOnCurve(long index, ksapi::IDrawingObjectPtr partner) {
    ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
    constr->SetConstraintType(ksConstraintTypeEnum::ksCPointOnCurve);
    constr->SetIndex(index);
    constr->SetPartners({ partner });
    return constr->Create();
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

bool ConstraintsCreator::equalRadius(ksapi::IDrawingObjectPtr partner) {
    ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
    constr->SetConstraintType(ksConstraintTypeEnum::ksCEqualRadius);
    constr->SetPartners({ partner });
    return constr->Create();
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

bool ConstraintsCreator::mergePoints(long index, ksapi::IDrawingObjectPtr partner, long partnerIndex) {
    ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
    constr->SetConstraintType(ksConstraintTypeEnum::ksCMergePoints);
    constr->SetIndex(index);
    constr->SetPartners({ partner });
    constr->SetPartnerIndex(partnerIndex);
    return constr->Create();
}

bool ConstraintsCreator::dimWithVariable(_bstr_t expression) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCDimWithVariable;
    constraint->Expression = expression;
    return constraint->Create();
}

bool ConstraintsCreator::dimWithVariable(std::wstring_view expression) {
    ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
    constr->SetConstraintType(ksConstraintTypeEnum::ksCDimWithVariable);
    constr->SetExpression(expression.data());
    return constr->Create();
}

bool ConstraintsCreator::fixedDim() {
    if (m_drawingObject1) {
        kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
        constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCFixedDim;
        return constraint->Create();
    }
    else if (m_drawingObject) {
        ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
        constr->SetConstraintType(ksConstraintTypeEnum::ksCFixedDim);
        return constr->Create();
    }
}

bool ConstraintsCreator::tangentTwoCurves(kapi::IDrawingObjectPtr partner) {
    kapi::IParametriticConstraintPtr constraint(m_drawingObject1->NewConstraint());
    constraint->ConstraintType = kapi::ksConstraintTypeEnum::ksCTangentTwoCurves;
    constraint->Partner = static_cast<IDispatch*>(partner);
    return constraint->Create();
}

bool ConstraintsCreator::tangentTwoCurves(ksapi::IDrawingObjectPtr partner) {
    ksapi::IParametricConstraintPtr constr = m_drawingObject->NewConstraint();
    constr->SetConstraintType(ksConstraintTypeEnum::ksCTangentTwoCurves);
    constr->SetPartners({ partner });
    return constr->Create();
}
