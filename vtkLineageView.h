// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// .NAME vtkLineageView - Display a cell lineage hierarchy
//
// .SECTION Description
// vtkLineageView is a prototype class that displays cool stuff about
// cell lineage trees.
//
// .SECTION See Also
// vtkTreeMapView
//
// .SECTION Thanks
// Thanks to the Universe for making the space in which this class
// was constructed within.


#ifndef __vtkLineageView_h
#define __vtkLineageView_h

#include "vtkRenderView.h"
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.

class vtkAbstractGraph;
class vtkAnnotationLink;
class vtkElbowGraphToPolyData;
class vtkTreeLayoutStrategy;
class vtkGraphLayout;
class vtkDelaunay2D;
class vtkContourFilter;
class vtkSplineFilter;
class vtkCornerAnnotation;
class vtkPolyDataMapper;
class vtkAlgorithmOutput;
class vtkActor;
class vtkActor2D;
class vtkGlyph3D;
class vtkRenderWindowInteractor;
class vtkInteractorStyleImage;
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkLookupTable;
class vtkLabeledDataMapper;
class vtkDynamic2DLabelMapper;
class vtkEventForwarderCommand;
class vtkAlgorithmOutput;
class vtkSphereSource;
class vtkConeSource;
class vtkTree;
class vtkTreeAlgorithm;
class vtkSelection;
class vtkVisibleCellSelector;
class vtkExtractSelectedIds;
class vtkRenderWindowInteractor;
class vtkInteractorStyleSelect2D;
class vtkRenderedAreaPicker;
class vtkDataSetMapper;
class vtkGeometryFilter;
class vtkDynamic2DLabelMapper;
class vtkCellCenters;
class vtkTreeCollapseFilter;
class vtkTreeVertexToEdgeSelection;
class vtkThresholdPoints;
class vtkVertexGlyphFilter;

class vtkLineageView : public vtkRenderView 
{
public:
  static vtkLineageView *New();
  vtkTypeRevisionMacro(vtkLineageView, vtkRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void UpdateMappersForColorCoding(const char* iArray,
      int iMinValue, int iMaxValue, bool iScalarVisibility);

  // Description:
  // The name of the vertex field used for coloring the vertices
  virtual void SetVertexColorFieldName(const char *field);
  virtual char* GetVertexColorFieldName();
  
  // Description:
  // The name of the edge field used for coloring the edges
  virtual void SetEdgeColorFieldName(const char *field);
  virtual char* GetEdgeColorFieldName();
  
  // Description:
  // The name of the field used for labeling
  virtual void SetLabelFieldName(const char *field);
  virtual char* GetLabelFieldName();
  
  // Description:
  // These methods turn labeling on/off. Defaulted to off
  virtual void SetLabelsOn();
  virtual void SetLabelsOff();
  
  // Description:
  // The size of the font used for labeling
  virtual void SetFontSize(const int size);
  virtual int GetFontSize();
  
  // Description:
  // Set/Get the field to use for the edge weights.
  vtkSetStringMacro(EdgeWeightField);
  vtkGetStringMacro(EdgeWeightField);
  
  // Description:
  // Set whether to use radial layout.
  void SetRadialLayout(bool radial);
  bool GetRadialLayout() { return this->Radial; }
  
  // Description:
  // Set the radial layout angle.
  void SetRadialAngle(int angle);
  int GetRadialAngle();
  
  // Description:
  // Set the log spacing for the layout.
  void SetLogSpacingFactor(float spacing);
  
  // Description:
  // Turn back plane on/off. Defaulted to off
  virtual void SetBackPlane(bool state);
  
  // Description:
  // Turn isocontour on/off. Defaulted to on
  virtual void SetIsoContour(bool state);
  
  // Description:
  // Set the current time
  void SetCurrentTime(double time_value);

  // Description:
  // Set the elbow parameters.
  void SetElbow(int onOff);
  void SetElbowAngle(double value);

  // Description:
  // Change the mode of coloring edges by a scalar.
  void SetEdgeScalarVisibility(bool value);

  // Description:
  // Set the array name to use for the vertex distance.
  void SetDistanceArrayName(const char* name);

  // Description:
  // Block the updating to make things faster
  vtkSetClampMacro(BlockUpdate, int, 0, 1);
  vtkBooleanMacro(BlockUpdate, int);
  vtkGetMacro(BlockUpdate, int);

//BTX
  enum
    {
    COLLAPSE_MODE,
    SELECT_MODE
    };
//ETX

  // Description:
  // The interaction mode for the viewer.
  vtkSetMacro(SelectMode, int);
  vtkGetMacro(SelectMode, int);

  // Description:
  // Apply the theme to this view.
  virtual void ApplyViewTheme(vtkViewTheme* theme);

protected:
  vtkLineageView();
  ~vtkLineageView();
  
  // Description:
  // Connects the representation to the internal pipeline.
  virtual void AddRepresentationInternal(vtkDataRepresentation* rep);
  
  // Description:
  // Disconnects the representation from the internal pipeline.
  virtual void RemoveRepresentationInternal(vtkDataRepresentation* rep);
  
  // Description:
  // Called to process the user event from the interactor style.
  virtual void ProcessEvents(vtkObject* caller, unsigned long eventId, 
    void* callData);
  
  // Description:
  // Connects the selection link to the internal pipeline.
  virtual void SetAnnotationLink(vtkAnnotationLink* link);
  
  // Decsription:
  // Prepares the view for rendering.
  virtual void PrepareForRendering();

  //BTX
  vtkSmartPointer<vtkTreeLayoutStrategy>            TreeLayoutStrategy;
  vtkSmartPointer<vtkGraphLayout>                   TreeLayout;
  vtkSmartPointer<vtkDelaunay2D>                    MakePlane;
  vtkSmartPointer<vtkContourFilter>                 IsoContour;
  vtkSmartPointer<vtkSplineFilter>                  SmoothContour;
  vtkSmartPointer<vtkCornerAnnotation>              CornerAnnotation;
  vtkSmartPointer<vtkElbowGraphToPolyData>          TreeToPolyData;
  vtkSmartPointer<vtkElbowGraphToPolyData>          CollapseToPolyData;
  vtkSmartPointer<vtkSphereSource>                  SphereSource;
  vtkSmartPointer<vtkConeSource>                    ConeSource;
  vtkSmartPointer<vtkVertexGlyphFilter>             VertexGlyphs;
  vtkSmartPointer<vtkPolyDataMapper>                IsoLineMapper;
  vtkSmartPointer<vtkPolyDataMapper>                PlaneMapper;
  vtkSmartPointer<vtkPolyDataMapper>                GlyphMapper;
  vtkSmartPointer<vtkPolyDataMapper>                CollapseMapper;
  vtkSmartPointer<vtkActor>                         IsoActor;
  vtkSmartPointer<vtkActor>                         PlaneActor;
  vtkSmartPointer<vtkActor>                         GlyphActor;
  vtkSmartPointer<vtkActor>                         CollapseActor;
  vtkSmartPointer<vtkActor2D>                       LabelActor;
  vtkSmartPointer<vtkLookupTable>                   ColorLUT;
  vtkSmartPointer<vtkDynamic2DLabelMapper>          LabeledDataMapper;
  vtkSmartPointer<vtkVisibleCellSelector>           VisibleCellSelector;
  vtkSmartPointer<vtkExtractSelectedIds>            ExtractSelection;
  vtkSmartPointer<vtkGeometryFilter>                SelectionGeometry;
  vtkSmartPointer<vtkDataSetMapper>                 SelectionMapper;
  vtkSmartPointer<vtkActor>                         SelectionActor;
  vtkSmartPointer<vtkRenderedAreaPicker>            Picker;
  vtkSmartPointer<vtkCellCenters>                   CellCenters;
  vtkSmartPointer<vtkTreeCollapseFilter>            TreeCollapse;
  vtkSmartPointer<vtkTreeVertexToEdgeSelection>     TreeVertexToEdge;
  vtkSmartPointer<vtkGlyph3D>                       CollapsedNodes;
  vtkSmartPointer<vtkPolyDataMapper>                CollapsedGlyphMapper;
  vtkSmartPointer<vtkActor>                         CollapsedGlyphActor;
  vtkSmartPointer<vtkThresholdPoints>               CollapsedThreshold;
  //ETX
  
  // Description:
  // This intercepts events from the graph layout class 
  // and re-emits them as if they came from this class.
  vtkEventForwarderCommand *EventForwarder;
  unsigned long ObserverTag;

  int SelectMode;
  
private:

  // Internally used methods
  
  // Description:
  // Setup the internal pipeline for the graph layout view
  virtual void SetupPipeline();
  
  // Description:
  // The field to use for the edge weights
  char*  EdgeWeightField;
  
  // Description:
  // Whether to use radial layout.
  bool Radial;
  
  // Description:
  // The radial layout angle.
  int Angle;
  
  // Description:
  // The log spacing for the layout.
  float LogSpacing;
  
  // Description:
  // Keep track of the minimum, maximum, current time values
  double MinTime;
  double CurrentTime;
  double MaxTime;
  int BlockUpdate;

  vtkLineageView(const vtkLineageView&);  // Not implemented.
  void operator=(const vtkLineageView&);  // Not implemented.
};

#endif
