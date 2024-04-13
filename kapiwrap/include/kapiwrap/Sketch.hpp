#ifndef SKETCH_HPP
#define SKETCH_HPP

struct Sketch {
    kapi::ksEntityPtr entity;
    kapi::ksSketchDefinitionPtr definition;
    kapi::ksDocument2DPtr document2d;
    kapi::IKompasDocument2DPtr document2d_api7;
    kapi::IViewPtr view;
    kapi::IDrawingContainerPtr drawingContainer;

    Sketch(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, IDispatchPtr plane);

    void endEdit() const;
};

#endif // SKETCH_HPP
