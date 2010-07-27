// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkTreeVertexToEdgeSelection.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkTree.h"

#include <vtksys/stl/map>

class vtkTreeVertexToEdgeSelectionInternals
{
public:
  vtksys_stl::map<vtkIdType, vtkIdType> PedigreeToId;
};

vtkCxxRevisionMacro(vtkTreeVertexToEdgeSelection, "$Revision$");
vtkStandardNewMacro(vtkTreeVertexToEdgeSelection);

vtkTreeVertexToEdgeSelection::vtkTreeVertexToEdgeSelection()
{
  this->SetNumberOfInputPorts(2);
  this->Internals = new vtkTreeVertexToEdgeSelectionInternals;
  this->Tree = NULL;
  this->TreeMTime = 0;
}

vtkTreeVertexToEdgeSelection::~vtkTreeVertexToEdgeSelection()
{
  delete this->Internals;
}

void vtkTreeVertexToEdgeSelection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkTreeVertexToEdgeSelection::RequestData(
  vtkInformation* vtkNotUsed(request), 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  vtkSelection* input = vtkSelection::GetData(inputVector[0]);
  vtkTree* tree = vtkTree::GetData(inputVector[1]);
  vtkSelection* output = vtkSelection::GetData(outputVector);

  // Update mapping
  if (tree != this->Tree || tree->GetMTime() > this->TreeMTime)
    {
    this->Internals->PedigreeToId.clear();
    vtkIdTypeArray* ped = vtkIdTypeArray::SafeDownCast(
      tree->GetVertexData()->GetAbstractArray("PedigreeVertexId"));
    for (vtkIdType v = 0; v < tree->GetNumberOfVertices(); v++)
      {
      this->Internals->PedigreeToId[ped->GetValue(v)] = v;
      }
    this->Tree = tree;
    this->TreeMTime = tree->GetMTime();
    }

  vtkSelectionNode* node = input->GetNode(0);
  if (node)
    {
    vtkIdTypeArray* arr = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
    if (arr)
      {
      vtkSmartPointer<vtkIdTypeArray> outArr =
        vtkSmartPointer<vtkIdTypeArray>::New();
      vtkSmartPointer<vtkSelectionNode> outNode =
        vtkSmartPointer<vtkSelectionNode>::New();
      for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); i++)
        {
        vtkIdType pedigree = arr->GetValue(i);
        vtkIdType vertId = this->Internals->PedigreeToId[pedigree];
        vtkEdgeType edgeId = tree->GetParentEdge(vertId);
        if (edgeId.Id >= 0)
          {
          outArr->InsertNextValue(edgeId.Id);
          }
        }
      outNode->SetSelectionList(outArr);
      outNode->GetProperties()->Copy(node->GetProperties());
      outNode->SetContentType(vtkSelectionNode::INDICES);
      outNode->SetFieldType(vtkSelectionNode::CELL);
      output->AddNode(outNode);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkTreeVertexToEdgeSelection::FillInputPortInformation(
  int port, vtkInformation* info)
{
  // now add our info
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTree");
    }
  return 1;
}

