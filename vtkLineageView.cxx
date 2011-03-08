// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkLineageView.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAnnotationLink.h"
#include "vtkCamera.h"
#include "vtkCellCenters.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkCoordinate.h"
#include "vtkDataRepresentation.h"
#include "vtkDataSetMapper.h"
#include "vtkDynamic2DLabelMapper.h"
#include "vtkEventForwarderCommand.h"
#include "vtkExtractSelectedIds.h"
#include "vtkGeometryFilter.h"
#include "vtkGlyph3D.h"
#include "vtkIdTypeArray.h"
#include "vtkInteractorStyleRubberBand2D.h"
#include "vtkPointData.h"
#include "vtkRenderedAreaPicker.h"
#include "vtkSelection.h"
#include "vtkTree.h"
#include "vtkTreeLayoutStrategy.h"
#include "vtkTreeVertexToEdgeSelection.h"
#include "vtkTreeWriter.h"
#include "vtkVisibleCellSelector.h"
#include "vtkDelaunay2D.h"
#include "vtkContourFilter.h"
#include "vtkSplineFilter.h"
#include "vtkGraphLayout.h"
#include "vtkGraphLayoutStrategy.h"
#include "vtkElbowGraphToPolyData.h"
#include "vtkInformation.h"
#include "vtkLabeledDataMapper.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkTextProperty.h"
#include "vtkAlgorithmOutput.h"
#include "vtkConeSource.h"
#include "vtkThresholdPoints.h"
#include "vtkVertexGlyphFilter.h"

#include <set>

class vtkCoordinate;

vtkCxxRevisionMacro(vtkLineageView, "$Revision$");
vtkStandardNewMacro(vtkLineageView);

//----------------------------------------------------------------------------
vtkLineageView::vtkLineageView()
{
  this->TreeLayoutStrategy    = vtkSmartPointer<vtkTreeLayoutStrategy>::New();
  this->TreeLayout            = vtkSmartPointer<vtkGraphLayout>::New();
  this->MakePlane             = vtkSmartPointer<vtkDelaunay2D>::New();
  this->IsoContour            = vtkSmartPointer<vtkContourFilter>::New();
  this->SmoothContour         = vtkSmartPointer<vtkSplineFilter>::New();
  this->CollapseToPolyData    = vtkSmartPointer<vtkElbowGraphToPolyData>::New();
  this->IsoLineMapper         = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->PlaneMapper           = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->GlyphMapper           = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->CollapseMapper        = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->IsoActor              = vtkSmartPointer<vtkActor>::New();
  this->PlaneActor            = vtkSmartPointer<vtkActor>::New();
  this->GlyphActor            = vtkSmartPointer<vtkActor>::New();
  this->CollapseActor         = vtkSmartPointer<vtkActor>::New();
  this->LabelActor            = vtkSmartPointer<vtkActor2D>::New();
  this->ConeSource            = vtkSmartPointer<vtkConeSource>::New();
  this->VertexGlyphs          = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->ColorLUT              = vtkSmartPointer<vtkLookupTable>::New();
  this->LabeledDataMapper     = vtkSmartPointer<vtkDynamic2DLabelMapper>::New();
  this->VisibleCellSelector   = vtkSmartPointer<vtkVisibleCellSelector>::New();
  this->ExtractSelection      = vtkSmartPointer<vtkExtractSelectedIds>::New();
  this->SelectionGeometry     = vtkSmartPointer<vtkGeometryFilter>::New();
  this->SelectionMapper       = vtkSmartPointer<vtkDataSetMapper>::New();
  this->SelectionActor        = vtkSmartPointer<vtkActor>::New();
  this->Picker                = vtkSmartPointer<vtkRenderedAreaPicker>::New();
  this->CellCenters           = vtkSmartPointer<vtkCellCenters>::New();
  this->CollapsedNodes        = vtkSmartPointer<vtkGlyph3D>::New();
  this->CollapsedGlyphMapper  = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->CollapsedGlyphActor   = vtkSmartPointer<vtkActor>::New();
  this->TreeVertexToEdge      = vtkSmartPointer<vtkTreeVertexToEdgeSelection>::New();
  this->CollapsedThreshold    = vtkSmartPointer<vtkThresholdPoints>::New();
  this->EdgeWeightField       = 0;
  this->CurrentTime           = 0.0;

  this->BlockUpdate = 0;
  this->SelectMode       = vtkLineageView::SELECT_MODE;

  // Set up eventforwarder
  this->EventForwarder = vtkEventForwarderCommand::New();
  this->EventForwarder->SetTarget(this);

  // Replace the interactor style
  this->SetInteractionModeTo2D();

  // Set up some the default label parameters
  this->LabeledDataMapper->SetFieldDataName("name");
  this->LabeledDataMapper->SetLabelModeToLabelFieldData();
  this->LabeledDataMapper->GetLabelTextProperty()->SetColor(0,0,0);
  this->LabeledDataMapper->GetLabelTextProperty()->SetJustificationToCentered();
  this->LabeledDataMapper->GetLabelTextProperty()->SetFontSize(14);

  this->ConeSource->SetRadius(0.01);
  this->ConeSource->SetHeight(0.01);
  this->ConeSource->SetDirection(0, 1, 0);

  this->CollapsedNodes->ScalingOff();

  this->TreeLayoutStrategy->SetAngle(360);
  this->TreeLayoutStrategy->SetRadial(true);
  this->TreeLayoutStrategy->SetLogSpacingValue(1);

  this->TreeLayout->SetLayoutStrategy(this->TreeLayoutStrategy);

  this->IsoContour->SetValue(0, this->CurrentTime);
  this->IsoActor->GetProperty()->SetLineWidth(5);

  this->SmoothContour->SetSubdivideToLength();
  this->SmoothContour->SetLength(.01);

  // Okay setup the internal pipeline
  this->SetupPipeline();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkLineageView::~vtkLineageView()
{
  this->SetEdgeWeightField(0);

  this->EventForwarder->Delete();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetCurrentTime(double time_value)
{
  this->CurrentTime = time_value;
  this->IsoContour->SetValue(0, this->CurrentTime);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetFontSize(const int size)
{
  this->LabeledDataMapper->GetLabelTextProperty()->SetFontSize(size);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
int vtkLineageView::GetFontSize()
{
  return this->LabeledDataMapper->GetLabelTextProperty()->GetFontSize();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetLabelFieldName(const char *field)
{
  this->LabeledDataMapper->SetFieldDataName(field);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
char* vtkLineageView::GetLabelFieldName()
{
  return this->LabeledDataMapper->GetFieldDataName();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetLabelsOn()
{
  this->LabelActor->VisibilityOn();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetLabelsOff()
{
  this->LabelActor->VisibilityOff();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetRadialLayout(bool radial)
{
  this->Radial = radial;
  this->TreeLayoutStrategy->SetRadial(this->Radial);
  this->Renderer->ResetCamera();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetBackPlane(bool state)
{
  if (state)
    {
    this->PlaneActor->VisibilityOn();
    }
  else
    {
    this->PlaneActor->VisibilityOff();
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetIsoContour(bool state)
{
  if (state)
    {
    this->IsoActor->VisibilityOn();
    }
  else
    {
    this->IsoActor->VisibilityOff();
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetRadialAngle(int angle)
{
  this->Angle = angle;
  this->TreeLayoutStrategy->SetAngle(this->Angle);
  this->Renderer->ResetCamera();
}

//----------------------------------------------------------------------------
int vtkLineageView::GetRadialAngle()
{
  return this->Angle;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetLogSpacingFactor(float spacing)
{
  this->LogSpacing = spacing;
  this->TreeLayoutStrategy->SetLogSpacingValue(this->LogSpacing);
  this->Renderer->ResetCamera();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetupPipeline()
{
  // Set various properties
  this->Renderer->SetBackground(0.3, 0.3, 0.3);
  this->ColorLUT->SetHueRange(0.667, 0.0);
  this->ColorLUT->Build();

  // Wire up the pipeline

  // Set the input to NULL and turn the
  // visibility of the actors off for now.
  // When SetInput() is called by the application
  // the input is set and the actors are turned on
  this->TreeLayout->SetInputConnection(NULL);
  this->IsoActor->PickableOff();
  this->PlaneActor->PickableOff();
  this->GlyphActor->PickableOff();
  this->CollapsedGlyphActor->PickableOn();
  this->LabelActor->PickableOff();
  this->SelectionActor->PickableOff();
  this->CollapseActor->PickableOn();
  this->PlaneActor->VisibilityOff();

  // Convert the laid out tree to poly data
  this->CollapseToPolyData->SetInputConnection(0, this->TreeLayout->GetOutputPort(0));

  // Set up glyphs at the tree nodes
  this->VertexGlyphs->SetInputConnection(0, this->CollapseToPolyData->GetOutputPort(0));

  // Set the glyph for the collapsed subtree
  this->CollapsedThreshold->SetInputConnection(0,
    this->CollapseToPolyData->GetOutputPort(0));
  this->CollapsedThreshold->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_POINTS_THEN_CELLS,
    "Collapsed");
  this->CollapsedThreshold->ThresholdByUpper(0.5);

  this->CollapsedNodes->SetInputConnection(0,
    this->CollapsedThreshold->GetOutputPort(0));
  this->CollapsedNodes->SetInputConnection(1, this->ConeSource->GetOutputPort(0));

  // Set up the back plane
  this->MakePlane->SetInputConnection(0, this->CollapseToPolyData->GetOutputPort(0));

  // Set up the iso contour (of the back plane)
  this->IsoContour->SetInputConnection(0, this->MakePlane->GetOutputPort(0));
  this->SmoothContour->SetInputConnection(0, this->IsoContour->GetOutputPort(0));

  // Set up label locations
  this->CellCenters->SetInputConnection(0, this->CollapseToPolyData->GetOutputPort(0));

  // Set up initial selection
  vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
  vtkSmartPointer<vtkSelectionNode> node = vtkSmartPointer<vtkSelectionNode>::New();
  vtkSmartPointer<vtkIdTypeArray> list = vtkSmartPointer<vtkIdTypeArray>::New();
  list->SetName("PedigreeVertexId");
  node->SetSelectionList(list);
  node->SetContentType(vtkSelectionNode::INDICES);
  node->SetFieldType(vtkSelectionNode::POINT);
  selection->AddNode(node);
  this->TreeVertexToEdge->SetInput(0, selection);

  // Set up selection
  this->TreeVertexToEdge->SetInputConnection(1, this->TreeLayout->GetOutputPort(0));
  this->ExtractSelection->SetInputConnection(0, this->CollapseToPolyData->GetOutputPort(0));
  this->ExtractSelection->SetInputConnection(1, this->TreeVertexToEdge->GetOutputPort(0));
  this->SelectionGeometry->SetInputConnection(0, this->ExtractSelection->GetOutputPort(0));

  // Set up mapper inputs
  this->IsoLineMapper->SetInputConnection(0, this->SmoothContour->GetOutputPort(0));
  this->PlaneMapper->SetInputConnection(0, this->MakePlane->GetOutputPort(0));
  this->GlyphMapper->SetInputConnection(0, this->VertexGlyphs->GetOutputPort(0));
  this->LabeledDataMapper->SetInputConnection(this->CollapseToPolyData->GetOutputPort());
  this->SelectionMapper->SetInputConnection(this->SelectionGeometry->GetOutputPort());
  this->CollapseMapper->SetInputConnection(this->CollapseToPolyData->GetOutputPort());
  this->CollapsedGlyphMapper->SetInputConnection(this->CollapsedNodes->GetOutputPort());

  // Set up mapper parameters
  this->SelectionMapper->SetScalarVisibility(false);
  this->CollapsedGlyphMapper->SetScalarVisibility(false);
  // Set up mapper which requiere LUT
  this->PlaneMapper->ColorByArrayComponent(0, 0);
  this->PlaneMapper->SetScalarRange( 0, 99);
  this->GlyphMapper->SetLookupTable(ColorLUT);
  this->GlyphMapper->SetScalarRange( 0, 99);
  this->IsoLineMapper->SetLookupTable(ColorLUT);
  this->IsoLineMapper->SetScalarRange( 0, 99);
  this->CollapseMapper->SetLookupTable(ColorLUT);
  this->CollapseMapper->SetScalarRange( 0, 99);

  // Set mappers to actors
  this->IsoActor->SetMapper(this->IsoLineMapper);
  this->PlaneActor->SetMapper(this->PlaneMapper);
  this->GlyphActor->SetMapper(this->GlyphMapper);
  this->GlyphActor->GetProperty()->SetPointSize(3.0);
  this->GlyphActor->SetPosition(0.0, 0.0, 0.2);
  this->CollapsedGlyphActor->SetMapper(this->CollapsedGlyphMapper);
  this->LabelActor->SetMapper(this->LabeledDataMapper);
  this->LabelActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->SelectionActor->SetMapper(this->SelectionMapper);
  this->SelectionActor->GetProperty()->SetColor(1.0, 0.0, 1.0);
  this->SelectionActor->GetProperty()->SetLineWidth(3);
  this->SelectionActor->SetPosition(0.0, 0.0, 0.1);
  this->CollapseActor->SetMapper(this->CollapseMapper);
  this->CollapseActor->GetProperty()->SetColor(0.0, 0.0, 0.0);

  // Add actors to renderer
  this->Renderer->SetBackground(1.0, 1.0, 1.0);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetVertexColorFieldName(const char *field)
{
  // Sanity Check
  if (!strcmp(field,"")) return;
  if (!strcmp(field,"No Filter")) return;

  this->GlyphMapper->SetScalarModeToUsePointFieldData();
  this->GlyphMapper->SelectColorArray(field);

  // Okay now get the range of the data field
  double range[2];
  this->CollapseToPolyData->Update();
  vtkDataArray *array =
    this->CollapseToPolyData->GetOutput()->GetPointData()->GetArray(field);
  if (array)
    {
    array->GetRange(range);
    this->GlyphMapper->SetScalarRange( range[0], range[1] );
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetEdgeColorFieldName(const char *field)
{
  // Sanity Check
  if (!strcmp(field,"")) return;
  if (!strcmp(field,"No Filter")) return;

  this->CollapseMapper->SetScalarModeToUsePointFieldData();
  this->CollapseMapper->SelectColorArray(field);
  this->PlaneMapper->SetScalarModeToUsePointFieldData();
  this->PlaneMapper->SelectColorArray(field);
  this->IsoLineMapper->SetScalarModeToUsePointFieldData();
  this->IsoLineMapper->SelectColorArray(field);

  // Okay now get the range of the data field
  double range[2];
  this->CollapseToPolyData->Update();
  vtkDataArray *array =
    this->CollapseToPolyData->GetOutput()->GetPointData()->GetArray(field);
  if (array)
    {
    array->GetRange(range);
    this->CollapseMapper->SetScalarRange( range[0], range[1] );
    this->PlaneMapper->SetScalarRange( range[0], range[1] );
    this->IsoLineMapper->SetScalarRange( range[0], range[1] );
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetEdgeScalarVisibility(bool value)
{
  this->CollapseMapper->SetScalarVisibility(value);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
char* vtkLineageView::GetVertexColorFieldName()
{
  return this->GlyphMapper->GetArrayName();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
char* vtkLineageView::GetEdgeColorFieldName()
{
  return this->CollapseMapper->GetArrayName();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetElbow(int onOff)
{
  this->CollapseToPolyData->SetElbow(onOff);
  this->Renderer->ResetCamera();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetElbowAngle(double value)
{
  this->CollapseToPolyData->SetFactor(value);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetDistanceArrayName(const char* name)
{
  this->TreeLayoutStrategy->SetDistanceArrayName(name);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Apply the theme to this view.
void vtkLineageView::ApplyViewTheme(vtkViewTheme* theme)
{
  this->Superclass::ApplyViewTheme(theme);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Connects the algorithm output to the internal pipeline.
void vtkLineageView::AddRepresentationInternal(vtkDataRepresentation* rep)
{
  if (true)
    {
    this->TreeLayout->SetInputConnection(rep->GetInputConnection());

    this->Renderer->AddActor(this->IsoActor);
    this->Renderer->AddActor(this->GlyphActor);
    this->Renderer->AddActor(this->LabelActor);
    this->Renderer->AddActor(this->SelectionActor);
    this->Renderer->AddActor(this->PlaneActor);
    this->Renderer->AddActor(this->CollapseActor);
    this->Renderer->ResetCamera();
    }
  else
    {
    vtkErrorMacro("This view only supports one representation.");
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Disconnects the algorithm output from the internal pipeline.
void vtkLineageView::RemoveRepresentationInternal(vtkDataRepresentation* rep)
{
  vtkAlgorithmOutput* conn = rep->GetInputConnection();
  if (0)
    {
    this->Renderer->RemoveActor(this->IsoActor);
    this->Renderer->RemoveActor(this->GlyphActor);
    this->Renderer->RemoveActor(this->LabelActor);
    this->Renderer->RemoveActor(this->SelectionActor);
    this->Renderer->RemoveActor(this->PlaneActor);
    this->Renderer->RemoveActor(this->CollapseActor);
    }
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::SetAnnotationLink(vtkAnnotationLink* link)
{
  this->TreeVertexToEdge->SetInputConnection(0, link->GetOutputPort(2));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Called to process the user event from the interactor style.
void vtkLineageView::ProcessEvents(vtkObject* caller, unsigned long eventId,
  void* callData)
{

  if (caller == this->GetInteractorStyle() && eventId == vtkCommand::SelectionChangedEvent)
    {
    unsigned int* rect = static_cast<unsigned int*>(callData);

    unsigned int screenMinX = rect[0];
    unsigned int screenMinY = rect[1];
    unsigned int screenMaxX = rect[2];
    unsigned int screenMaxY = rect[3];
    unsigned int delta = 1;
    if (screenMaxX == screenMinX || screenMaxY == screenMinY)
      {
      screenMinX = screenMinX < delta ? 0 : screenMinX - delta;
      screenMinY = screenMinY < delta ? 0 : screenMinY - delta;
      screenMaxX += delta;
      screenMaxY += delta;
      }

    this->VisibleCellSelector->SetRenderer(this->Renderer);
    this->VisibleCellSelector->SetArea(screenMinX, screenMinY, screenMaxX, screenMaxY);
    this->VisibleCellSelector->SetProcessorId(0);
    this->VisibleCellSelector->SetRenderPasses(0, 0, 0, 0, 1);
    this->VisibleCellSelector->Select();
/*
    std::set<vtkIdType> collapsedSet;
    vtkIdTypeArray* collapsedOld = this->TreeCollapse->GetCollapsedNodes();
    for (vtkIdType n = 0; n < collapsedOld->GetNumberOfTuples(); n++)
      {
      collapsedSet.insert(collapsedOld->GetValue(n));
      }

    // Convert to pedigree ids and add to selection
    vtkIdTypeArray* ids = vtkIdTypeArray::New();
    this->VisibleCellSelector->GetSelectedIds(ids);
    vtkIdTypeArray* selectedIds = vtkIdTypeArray::New();
    vtkTree* tree = vtkTree::SafeDownCast(this->TreeLayout->GetOutput());
    vtkIdTypeArray* ped = vtkIdTypeArray::SafeDownCast(
      tree->GetVertexData()->GetAbstractArray("PedigreeVertexId"));
    for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++)
      {
      vtkIdType edgeId = ids->GetValue(4*i+3);
      vtkIdType vertId = tree->GetTargetVertex(edgeId);
      vtkIdType pedId = ped->GetValue(vertId);
      selectedIds->InsertNextValue(pedId);
      if (collapsedSet.count(pedId) > 0)
        {
        collapsedSet.erase(pedId);
        }
      else
        {
        collapsedSet.insert(pedId);
        }
      }

    if (this->SelectMode == vtkLineageView::COLLAPSE_MODE)
      {
      vtkIdTypeArray* collapsedNew = vtkIdTypeArray::New();
      std::set<vtkIdType>::iterator it, itEnd;
      for (it = collapsedSet.begin(), itEnd = collapsedSet.end();
           it != itEnd; ++it)
        {
        collapsedNew->InsertNextValue(*it);
        std::cout << "Nodes to be collapsed: " << *it << std::endl;
        }
      this->TreeCollapse->SetCollapsedNodes(collapsedNew);
      collapsedNew->Delete();
      }
    else if (this->SelectMode == vtkLineageView::SELECT_MODE)
      {
      vtkSmartPointer<vtkSelection> selection = vtkSmartPointer<vtkSelection>::New();
      vtkSmartPointer<vtkSelectionNode> node = vtkSmartPointer<vtkSelectionNode>::New();
      node->SetContentType(vtkSelectionNode::INDICES);
      node->SetFieldType(vtkSelectionNode::VERTEX);
      node->SetSelectionList(selectedIds);
      selection->AddNode(node);

      // If this is a union selection, append the selection
      if (rect[4] == vtkInteractorStyleRubberBand2D::SELECT_UNION)
        {
        vtkSelection* oldSelection = this->GetRepresentation()->GetAnnotationLink()->GetCurrentSelection();
        selection->Union(oldSelection);
        }

      // Call select on the representation(s)
      this->GetRepresentation()->Select(this, selection);
      }

    ids->Delete();
    selectedIds->Delete();*/
    }
  else
    {
    Superclass::ProcessEvents(caller, eventId, callData);
    }

}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Decsription:
// Prepares the view for rendering.
void vtkLineageView::PrepareForRendering()
{
  // Make sure we have a representation.
  vtkDataRepresentation* rep = this->GetRepresentation();
  if (!rep)
    {
    return;
    }

  // Make sure the input connection is up to date.
  vtkAlgorithmOutput* conn = rep->GetInputConnection();
  this->TreeLayout->SetInputConnection(conn);

  // Make sure the selection link is up to date.
  vtkAnnotationLink* link = rep->GetAnnotationLink();
  if (this->TreeVertexToEdge->GetInputConnection(0, 0)->GetProducer() != link)
    {
    this->SetAnnotationLink(link);
    }

  this->Superclass::PrepareForRendering();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkLineageView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "EdgeWeightField: "
     << (this->EdgeWeightField ? this->EdgeWeightField : "(none)") << endl;
}
