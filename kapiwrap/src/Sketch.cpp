#include "kapiwrap/Sketch.hpp"

Sketch::Sketch(ksapi::IPartPtr part, ksapi::IPlane3DPtr plane)
{
	ksapi::IModelContainerPtr modelCont = part;
	ksapi::ISketchsPtr sketchs = modelCont->GetSketchs();

	ksapi::ISketchPtr sketch = sketchs->Add();
	sketch->SetPlane(plane);
	sketch->Update();
	m_sketch = sketch;
}

SketchEditor Sketch::edit()
{
	return SketchEditor(m_sketch);
}

void Sketch::endEdit() const
{
	if (m_sketch) {
		m_sketch->EndEdit();
	}
}

ksapi::ISketchPtr Sketch::getObject() const
{
	return m_sketch;
}

SketchEditor::SketchEditor(ksapi::ISketchPtr sketch):
	m_sketch(sketch)
{
	m_fragmentDoc = m_sketch->BeginEdit(false /*readOnly*/);
	ksapi::IViewsAndLayersManagerPtr layersMngr = m_fragmentDoc->GetViewsAndLayersManager();
	ksapi::IViewsPtr views = layersMngr->GetViews();
	m_activeView = views->GetActiveView();
	m_drawingCont = ksapi::IDrawingContainerPtr(m_activeView);
}

SketchEditor::~SketchEditor()
{
	m_sketch->EndEdit();
}

ksapi::IDrawingContainerPtr SketchEditor::getDrawingContainer() const
{
	return m_drawingCont;
}

ksapi::ISymbols2DContainerPtr SketchEditor::getSymbols2DContainer() const
{
	return ksapi::ISymbols2DContainerPtr(m_activeView);
}

ksapi::ILineSegmentPtr SketchEditor::addLineSegment(double x1, double y1, double x2, double y2) const
{
	ksapi::ILineSegmentsPtr lineSegments = m_drawingCont->GetLineSegments();
	ksapi::ILineSegmentPtr lineSeg = lineSegments->Add();
	lineSeg->SetX1(x1);
	lineSeg->SetY1(y1);
	lineSeg->SetX2(x2);
	lineSeg->SetY2(y2);
	lineSeg->Update();
	return lineSeg;
}

ksapi::IArcPtr SketchEditor::addArc(double xc, double yc, double x1, double y1, double x2, double y2, double radius, bool direction) const
{
	ksapi::IArcsPtr arcs = m_drawingCont->GetArcs();
	ksapi::IArcPtr arc = arcs->Add();
	arc->SetXc(xc);
	arc->SetYc(yc);
	arc->SetX1(x1);
	arc->SetY1(y1);
	arc->SetX2(x2);
	arc->SetY2(y2);
	arc->SetRadius(radius);
	arc->SetDirection(direction);
	arc->Update();
	return arc;
}

std::vector<ksapi::IDrawingObjectPtr> SketchEditor::addProjectionOf(ksapi::IModelObjectPtr object) const
{
	return m_sketch->AddProjectionOf(object);
}
