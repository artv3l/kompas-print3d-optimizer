#pragma once

#include <tuple>

#include <KsAPI.h>

#include "generic/ActionLock.hpp"

class SketchEditor final
{
public:
    SketchEditor(ksapi::ISketchPtr sketch);
    SketchEditor(const SketchEditor& other) = delete;
    SketchEditor(SketchEditor&& other) noexcept = default;

    ~SketchEditor();

    SketchEditor& operator=(const SketchEditor& other) = delete;
    SketchEditor& operator=(SketchEditor&& other) noexcept = default;

    ksapi::IDrawingContainerPtr getDrawingContainer() const;
    ksapi::ISymbols2DContainerPtr getSymbols2DContainer() const;

    std::vector<ksapi::IDrawingObjectPtr> addProjectionOf(ksapi::IModelObjectPtr object) const;
    ksapi::ILineSegmentPtr addLineSegment(double x1, double y1, double x2, double y2) const;
    ksapi::IArcPtr addArc(double xc, double yc, double x1, double y1, double x2, double y2, double radius, bool direction) const;

private:
    ksapi::ISketchPtr m_sketch;
    ksapi::IFragmentDocumentPtr m_fragmentDoc;
    ksapi::IViewPtr m_activeView;
    ksapi::IDrawingContainerPtr m_drawingCont;
};

class Sketch final
{
public:
    kapi::ksEntityPtr entity;
    kapi::ksSketchDefinitionPtr definition;
    kapi::ksDocument2DPtr document2d;
    kapi::IKompasDocument2DPtr document2d_api7;
    kapi::IViewPtr view;
    kapi::IDrawingContainerPtr drawingContainer;

    Sketch(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, IDispatchPtr plane);
    Sketch(ksapi::IPartPtr part, ksapi::IPlane3DPtr plane);

    SketchEditor edit();
    void endEdit() const;
    ksapi::ISketchPtr getObject() const;

private:
    ksapi::ISketchPtr m_sketch;
};
