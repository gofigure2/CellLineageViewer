// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkVolumeViewer.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLVolumeTextureMapper3D.h"
#include "vtkLODProp3D.h"
#include "vtkVolumeProperty.h"
#include "vtkXMLImageDataReader.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkTubeFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkAlgorithmOutput.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkLookupTable.h"
#include "vtkLabeledDataMapper.h"
#include "vtkDynamic2DLabelMapper.h"
#include "vtkEventForwarderCommand.h"
#include "vtkAlgorithmOutput.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkImageData.h"
#include "vtkCornerAnnotation.h"


vtkCxxRevisionMacro(vtkVolumeViewer, "$Revision$");
vtkStandardNewMacro(vtkVolumeViewer);


//----------------------------------------------------------------------------
vtkVolumeViewer::vtkVolumeViewer()
{
  this->RenderWindow     = NULL;
  this->Mapper           = vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D>::New();
  this->MapperLOD        = vtkSmartPointer<vtkOpenGLVolumeTextureMapper3D>::New();
  this->Volume           = vtkSmartPointer<vtkLODProp3D>::New();
  this->Property         = vtkSmartPointer<vtkVolumeProperty>::New();
  this->Opacity          = vtkSmartPointer<vtkPiecewiseFunction>::New();
  this->Color            = vtkSmartPointer<vtkColorTransferFunction>::New();
  this->CornerAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
  this->SelectionMapper  = vtkSmartPointer<vtkPolyDataMapper>::New();             ; 
  this->SelectionActor   = vtkSmartPointer<vtkActor>::New();
  this->InteractorStyle  = vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->Renderer         = vtkSmartPointer<vtkRenderer>::New();
  this->SphereSource     = vtkSmartPointer<vtkSphereSource>::New();
  this->VertexGlyphs     = vtkSmartPointer<vtkGlyph3D>::New();
  this->ColorLUT         = vtkSmartPointer<vtkLookupTable>::New();
  
  // Set up eventforwarder
  this->EventForwarder = vtkEventForwarderCommand::New();
  this->EventForwarder->SetTarget(this);
  
  
  // Set up some the default parameters
  this->Volume->SetScale(.005, .005, .005);
  this->Volume->SetPosition(1, -1, 0);
   
  //this->Mapper->SetSampleDistance(0.2);
  //this->MapperLOD->SetSampleDistance(0.5);


  this->Opacity->AddPoint(0, 0);
  this->Opacity->AddPoint(23, 0);
  this->Opacity->AddPoint(25, 0.05);
  this->Opacity->AddPoint(40, 0.05);
  this->Opacity->AddPoint(41, 0.1);
  this->Opacity->AddPoint(75, 0.1);
  this->Opacity->AddPoint(76, 0.2);
  this->Opacity->AddPoint(90, 0.2);
  this->Opacity->AddPoint(91, 0.5);

  this->Color->AddRGBPoint(  0, 0, 0, 1);
  this->Color->AddRGBPoint( 40, 0, 0, 1);
  this->Color->AddRGBPoint( 41, 0, 1, 0);
  this->Color->AddRGBPoint( 75, 0, 1, 0);
  this->Color->AddRGBPoint( 76, 1, 1, 0);
  this->Color->AddRGBPoint( 90, 1, 1, 0);
  this->Color->AddRGBPoint( 91, 1, 0, 0);
  this->Color->AddRGBPoint(255, 1, 0, 0);

  this->Property->SetColor(this->Color);
  this->Property->SetScalarOpacity(this->Opacity);
  this->Property->ShadeOn();
  this->Property->SetAmbient(0.2);
  this->Property->SetDiffuse(0.9);
  this->Property->SetSpecular(0.2);
  this->Property->SetInterpolationTypeToLinear();
  this->SphereSource->SetRadius(0.025); // Why? Given the current layout strategies
                                       // seems to work pretty good just hardcoding
  this->SphereSource->SetPhiResolution(6);
  this->SphereSource->SetThetaResolution(6);
  this->VertexGlyphs->ScalingOff();


  // Okay setup the internal pipeline
  this->SetupPipeline();

}

//----------------------------------------------------------------------------
vtkVolumeViewer::~vtkVolumeViewer()
{
  // Unregister vtk objects that were passed in
  this->SetRenderWindow(NULL);
  this->EventForwarder->Delete();
  
  // Smart pointers will handle the rest of
  // vtk pipeline objects :)
}

void vtkVolumeViewer::UpdateView(bool ResetCamera)
{
  if (this->RenderWindow)
    {
    if (ResetCamera)
      {
      this->Renderer->ResetCamera();
      }
    this->RenderWindow->Render();
    }
 }

void vtkVolumeViewer::SetInput(vtkImageData *image_data)
{
  if (image_data)
    {
    this->SetInputConnection(image_data->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0);
    }
}

void vtkVolumeViewer::SetInputConnection(vtkAlgorithmOutput* input) 
{
  this->Mapper->SetInputConnection(0, input);
  this->MapperLOD->SetInputConnection(0, input);

  this->Volume->VisibilityOn();
  this->SelectionActor->VisibilityOff();
  this->UpdateView();
};

// This method is a cut and paste of vtkCxxSetObjectMacro
// except for the AddRenderer and ResetCamera() call in the middle :)
void vtkVolumeViewer::SetRenderWindow(vtkRenderWindow *arg)
{
  if (arg != this->RenderWindow)
    {
    vtkRenderWindow *tmp = this->RenderWindow;
    this->RenderWindow = arg;
    if (this->RenderWindow != NULL)
      {
      this->RenderWindow->Register(this);
      
      // Set up last part of the pipeline
      this->RenderWindow->AddRenderer(this->Renderer);
      this->Renderer->ResetCamera();
      }
    if (tmp != NULL)
      {
      tmp->UnRegister(this);
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkVolumeViewer::SetupPipeline()
{

  // Set various properties
  this->Renderer->SetBackground(.3,.3,.3);
  this->ColorLUT->SetHueRange( 0.667, 0 );
  this->ColorLUT->Build();
 
  // Wire up the pipeline
 
  // Set the input to NULL and turn the 
  // visibility of the actors off for now.
  // When SetInput() is called by the application
  // the input is set and the actors are turned on
  this->Mapper->SetInputConnection(0, NULL);
  this->MapperLOD->SetInputConnection(0, NULL);
  this->Volume->AddLOD(this->Mapper, this->Property, 1.0);
  //this->Volume->AddLOD(this->MapperLOD, this->Property, 0.5);
  this->Volume->VisibilityOff();
  this->SelectionActor->VisibilityOff();

  // Set mappers to actors
  this->SelectionActor->SetMapper(this->SelectionMapper);

  // Add actors to renderer
  this->Renderer->AddViewProp(this->Volume);
  this->Renderer->AddActor(this->SelectionActor);
  this->Renderer->SetBackground(1,1,1);

}


void vtkVolumeViewer::SetSelectionColorFieldName(const char *field)
{
  // Sanity Check
  if (!strcmp(field,"")) return;
  if (!strcmp(field,"No Filter")) return;
  
  this->SelectionMapper->SetScalarModeToUsePointFieldData();
  this->SelectionMapper->SelectColorArray(field);

#if 0  
  // Okay now get the range of the data field
  double range[2]; 
  this->TreeToPolyData->Update();
  vtkDataArray *array =
    this->TreeToPolyData->GetOutput()->GetPointData()->GetArray(field);
  if (array)
    {
    array->GetRange(range);
    this->SelectionMapper->SetScalarRange( range[0], range[1] );
    } 
#endif

  if (this->RenderWindow)
    {
    this->RenderWindow->GetInteractor()->Render();
    }
}

char* vtkVolumeViewer::GetSelectionColorFieldName()
{
  return this->SelectionMapper->GetArrayName();
}



//----------------------------------------------------------------------------
void vtkVolumeViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "RenderWindow: " << (this->RenderWindow ? "" : "(none)") << endl;
  if (this->RenderWindow)
    {
    this->RenderWindow->PrintSelf(os,indent.GetNextIndent());
    }
  
  os << indent << "SelectionMapper: " << (this->SelectionMapper ? "" : "(none)") << endl;
  if (this->SelectionMapper)
    {
    this->SelectionMapper->PrintSelf(os,indent.GetNextIndent()); 
    }
    
  os << indent << "SphereSource: " << (this->SphereSource ? "" : "(none)") << endl;
  if (this->SphereSource)
    {
    this->SphereSource->PrintSelf(os,indent.GetNextIndent()); 
    }
    
  os << indent << "VertexGlyphs: " << (this->VertexGlyphs ? "" : "(none)") << endl;
  if (this->VertexGlyphs)
    {
    this->VertexGlyphs->PrintSelf(os,indent.GetNextIndent()); 
    }
      
  os << indent << "Renderer: " << (this->Renderer ? "" : "(none)") << endl;
  if (this->Renderer)
    {
    this->Renderer->PrintSelf(os,indent.GetNextIndent());
    }
  
  os << indent << "SelectionActor: " << (this->SelectionActor ? "" : "(none)") << endl;
  if (this->SelectionActor && this->RenderWindow)
    {
    this->SelectionActor->PrintSelf(os,indent.GetNextIndent());
    }
  
  os << indent << "InteractorStyle: " << (this->InteractorStyle ? "" : "(none)") << endl;
  if (this->InteractorStyle)
    {
    this->InteractorStyle->PrintSelf(os,indent.GetNextIndent());
    }
      
}
