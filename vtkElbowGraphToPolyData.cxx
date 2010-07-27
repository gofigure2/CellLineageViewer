// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkElbowGraphToPolyData.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"

#include <vtksys/stl/map>

vtkCxxRevisionMacro(vtkElbowGraphToPolyData, "$Revision$");
vtkStandardNewMacro(vtkElbowGraphToPolyData);

class vtkElbowGraphToPolyDataInternal
{
public:
  typedef vtksys_stl::map<vtkIdType, vtkIdType> MapIdTypeToIdType;
  MapIdTypeToIdType PointMapping;
  vtkIdType AddEdge(vtkGraph* input, vtkPolyData* output, vtkIdType edgeId,
    double factor);
  vtkIdType StorePoint(double point[3], vtkIdType inIndex, vtkPoints* outputPoints,
    vtkGraph* inputGraph, vtkPolyData* outputPD, vtkIdType copyPoint,
    vtkIdType copyNames, int removeLabel);
  void Initialize();
  vtkStringArray* InputNamesArray;
  vtkStringArray* OutputNamesArray;
};

vtkIdType vtkElbowGraphToPolyDataInternal::AddEdge(vtkGraph*
  input, vtkPolyData* output, vtkIdType edgeId, double factor)
{
  // Get input and output components
  vtkPoints* inputPts = input->GetPoints();
  vtkPoints* outputPts = output->GetPoints();
  vtkCellArray* lines = output->GetLines();

  // For the elbow 
  vtkIdType points[2];
  points[0] = input->GetSourceVertex(edgeId); 
  points[1] = input->GetTargetVertex(edgeId); 

  // Calculate middle points
  double pointSource[3];
  double pointTarget[3];
  double midPoint[3];
  //double inPointSource[3];
  //double inPointTarget[3];

  inputPts->GetPoint(points[0], pointSource);
  inputPts->GetPoint(points[1], pointTarget);

  //double invFactor = 1 - factor;
  
  double xFactor = 0;
  double yFactor = 0;
#define myABS(x) (((x) < 0)?(-(x)):(x))
  if ( factor >= 0 )
    {
    xFactor = factor;
    yFactor = 1;
    }
  else
    {
    xFactor = 1;
    yFactor = myABS(factor);
    }

  
  midPoint[0] = (pointSource[0]*xFactor + pointTarget[0]*(1-xFactor));
  midPoint[1] = (pointSource[1]*yFactor + pointTarget[1]*(1-yFactor));
  midPoint[2] = (pointSource[2] + pointTarget[2])/2;

  int removeInLabel = 1;
  int removeMidLabel = 0;
  int removeOutLabel = 1;

  // Copy the points to the target
  vtkIdType outputPolyLine[4];
  outputPolyLine[0] = this->StorePoint(pointSource, points[0], outputPts, input, output, points[0], points[0], removeInLabel);
  outputPolyLine[1] = this->StorePoint(midPoint,    -1       , outputPts, input, output, points[0], points[1], removeMidLabel);
  outputPolyLine[2] = this->StorePoint(pointTarget, points[1], outputPts, input, output, points[1], points[1], removeOutLabel);
  return lines->InsertNextCell(3, outputPolyLine);
}

vtkIdType vtkElbowGraphToPolyDataInternal::StorePoint(double point[3],
  vtkIdType inIndex, vtkPoints* outputPoints, vtkGraph* inputGraph,
  vtkPolyData* outputPD, vtkIdType copyPoint, vtkIdType copyNames, int removeLabel)
{
  vtkElbowGraphToPolyDataInternal::MapIdTypeToIdType::iterator cpointIt
    = this->PointMapping.find(inIndex);
  if ( cpointIt != this->PointMapping.end() )
    {
    return cpointIt->second;
    }
  vtkIdType newPtIdx = outputPoints->InsertNextPoint(point);
  if ( inIndex >= 0 )
    {
    this->PointMapping[inIndex] = newPtIdx;
    }
  outputPD->GetPointData()->CopyData(inputGraph->GetVertexData(),copyPoint,newPtIdx);
  if ( this->OutputNamesArray )
    {
    if ( removeLabel )
      {
      this->OutputNamesArray->SetValue(newPtIdx, "");
      }
    else if ( this->InputNamesArray )
      {
      this->OutputNamesArray->SetValue(newPtIdx, this->InputNamesArray->GetValue(copyNames));
      }
    }
  return newPtIdx;
}

void vtkElbowGraphToPolyDataInternal::Initialize()
{
  this->PointMapping.erase(this->PointMapping.begin(), this->PointMapping.end());
  this->InputNamesArray = 0;
  this->OutputNamesArray = 0;
}

vtkElbowGraphToPolyData::vtkElbowGraphToPolyData()
{
  this->Internals = new vtkElbowGraphToPolyDataInternal;
  this->Factor = 0;
  this->Elbow = 0;
}

vtkElbowGraphToPolyData::~vtkElbowGraphToPolyData()
{
  delete this->Internals;
  this->Internals = 0;
}

void vtkElbowGraphToPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Factor " << this->Factor << endl;
  os << indent << "Elbow " << (this->Elbow?"ON":"OFF") << endl;
}

int vtkElbowGraphToPolyData::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if ( !this->Elbow )
    {
    return this->Superclass::RequestData(request, inputVector, outputVector);
    }
  this->Internals->Initialize();
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkGraph *input = vtkGraph::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkIdType numPts = input->GetNumberOfVertices();
  output->GetPointData()->CopyAllocate(input->GetVertexData(), 4*numPts);

  vtkDataArray* edgeGhostLevels = vtkDataArray::SafeDownCast(
    input->GetVertexData()->GetAbstractArray("vtkGhostLevels"));

  vtkCellArray* newLines = vtkCellArray::New();
  output->SetLines(newLines);
  newLines->Delete();

  vtkPoints* outputPoints = vtkPoints::New();
  output->SetPoints(outputPoints);
  outputPoints->Delete();

  vtkIdType numEdges = input->GetNumberOfEdges();
  vtkDataSetAttributes* inputEdgeData = input->GetEdgeData();
  vtkCellData* outputCellData = output->GetCellData();
  outputCellData->CopyAllocate(inputEdgeData);

  newLines->Allocate(newLines->EstimateSize(numEdges, 2));

  this->Internals->InputNamesArray = vtkStringArray::SafeDownCast(
    input->GetVertexData()->GetAbstractArray("name"));
  this->Internals->OutputNamesArray = vtkStringArray::SafeDownCast(
    output->GetPointData()->GetAbstractArray("name"));

  //vtkIdType points[2];

  if (edgeGhostLevels == NULL)
    {
    // Send the data to output.
    for (vtkIdType i = 0; i < numEdges; i++)
      {
      this->Internals->AddEdge(input, output, i, this->Factor);
      }

    // Cells correspond to edges, so pass the cell data along.
    output->GetCellData()->PassData(input->GetEdgeData());
    }
  else
    {
    // Only create lines for non-ghost edges
    for (vtkIdType i = 0; i < numEdges; i++)
      {
      if (edgeGhostLevels->GetComponent(i, 0) == 0) 
        {
        vtkIdType ind = this->Internals->AddEdge(input, output, i, this->Factor);
        outputCellData->CopyData(inputEdgeData, i, ind);
        }
      } 
    }
  output->Squeeze();

  return 1;
}

