#ifndef SKETCH_HPP
#define SKETCH_HPP

struct Sketch {
    ksEntityPtr entity;
    ksSketchDefinitionPtr definition;
    ksDocument2DPtr document2d;
    IKompasDocument2DPtr document2d_api7;
    IViewPtr view;
    IDrawingContainerPtr drawingContainer;

    Sketch(KompasObjectPtr kompas, ksPartPtr part, IDispatchPtr plane);

    void endEdit() const;
};

#endif // SKETCH_HPP
