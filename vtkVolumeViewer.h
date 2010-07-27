// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// .NAME vtkVolumeViewer - Volume rendering view
//
// .SECTION Description
// vtkVolumeViewer is a prototype class that displays a volume rendering
// of structured data.
//
// .SECTION See Also
// vtkImageViewer vtkImageViewer2
//
// .SECTION Thanks
// Thanks to the Universe for making the space in which this class
// was constructed within.


#ifndef __vtkVolumeViewer_h
#define __vtkVolumeViewer_h

#include "vtkObject.h"
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
    
class vtkOpenGLVolumeTextureMapper3D;
class vtkLODProp3D;
class vtkVolumeProperty;
class vtkXMLImageDataReader;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkTubeFilter;
class vtkPolyDataMapper;
class vtkAlgorithmOutput;
class vtkActor;
class vtkActor2D;
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
class vtkGlyph3D;
class vtkColorLUT;
class vtkImageData;
class vtkCornerAnnotation;

class vtkVolumeViewer : public vtkObject 
{
public:
  static vtkVolumeViewer *New();
  vtkTypeRevisionMacro(vtkVolumeViewer,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set the input data to the viewer.
  virtual void SetInput(vtkImageData *image_data);
  
  // Description:
  // Set the input data to the viewer.
  virtual void SetInputConnection(vtkAlgorithmOutput* input); 

  // Description:
  // Set your own renderwindow
  virtual void SetRenderWindow(vtkRenderWindow *arg);
  vtkGetObjectMacro(RenderWindow, vtkRenderWindow);
  
  // Description:
  // The name of the field used for coloring the selections
  virtual void SetSelectionColorFieldName(const char *field);
  virtual char* GetSelectionColorFieldName();

  // Description:
  // Tell the viewer to explicity update the view
  void UpdateView(bool ResetCamera=true);

protected:
  vtkVolumeViewer();
  ~vtkVolumeViewer();
  
  vtkRenderWindow*                                RenderWindow;
  //BTX
  vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D> Mapper;
  vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D> MapperLOD;
  vtkSmartPointer<vtkLODProp3D>                   Volume;
  vtkSmartPointer<vtkVolumeProperty>              Property;
  vtkSmartPointer<vtkPiecewiseFunction>           Opacity;
  vtkSmartPointer<vtkColorTransferFunction>       Color;
  vtkSmartPointer<vtkCornerAnnotation>            CornerAnnotation;
  vtkSmartPointer<vtkSphereSource>                SphereSource;
  vtkSmartPointer<vtkGlyph3D>                     VertexGlyphs;
  vtkSmartPointer<vtkInteractorStyleImage>        InteractorStyle;
  vtkSmartPointer<vtkRenderer>                    Renderer;
  vtkSmartPointer<vtkPolyDataMapper>              SelectionMapper;
  vtkSmartPointer<vtkActor>                       SelectionActor;
  vtkSmartPointer<vtkLookupTable>                 ColorLUT;
  //ETX
  
  // Description:
  // This intercepts events from the graph layout class 
  // and re-emits them as if they came from this class.
  vtkEventForwarderCommand *EventForwarder;
  unsigned long ObserverTag;
  
private:

  // Internally used methods
  
  // Description:
  // Setup the internal pipeline for the graph layout view
  virtual void SetupPipeline();
  
  
  vtkVolumeViewer(const vtkVolumeViewer&);  // Not implemented.
  void operator=(const vtkVolumeViewer&);  // Not implemented.
};

#endif


