// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkTreeCollapseFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDataArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkOutEdgeIterator.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"
#include "vtkVariant.h"

#include <assert.h>
#include <vtksys/stl/set>

vtkCxxRevisionMacro(vtkTreeCollapseFilter, "$Revision$");
vtkStandardNewMacro(vtkTreeCollapseFilter);
vtkCxxSetObjectMacro(vtkTreeCollapseFilter, CollapsedNodes, vtkIdTypeArray);

vtkTreeCollapseFilter::vtkTreeCollapseFilter()
{
  this->CollapsedNodes = 0;
}

vtkTreeCollapseFilter::~vtkTreeCollapseFilter()
{
  this->SetCollapsedNodes(0);
}

void vtkTreeCollapseFilterCollapsedDFS(vtkTree* input,
  vtkIdType idx, vtkIdType parent, vtkMutableDirectedGraph* output, 
  const vtksys_stl::set<vtkIdType>& collapsedNodes, vtkPoints* pts,
  vtkIdTypeArray* ped, vtkCharArray* collapsedArray)
{
  vtkIdType newChild = -1;
  if ( parent == -1 )
    {
    newChild = output->AddVertex();
    }
  else
    {
    newChild = output->AddChild(parent);
    vtkEdgeType newEdge = output->AddEdge(parent, newChild);
    vtkEdgeType oldEdge = input->GetParentEdge(idx);
    output->GetEdgeData()->CopyData(input->GetEdgeData(),oldEdge.Id,newEdge.Id);
    }
  output->GetVertexData()->CopyData(input->GetVertexData(),idx,newChild);
  double pt[3];
  input->GetPoint(idx, pt);
  pts->InsertPoint(newChild, pt);

  // If it is a collapsed node, skip traversal
  vtkIdType pedId = ped->GetValue(idx);
  if ( collapsedNodes.find(pedId) == collapsedNodes.end() )
    {
    vtkSmartPointer<vtkOutEdgeIterator> it = vtkSmartPointer<vtkOutEdgeIterator>::New();
    input->GetOutEdges(idx, it);
    while ( it->HasNext() )
      {
      vtkOutEdgeType e = it->Next();
      vtkTreeCollapseFilterCollapsedDFS(input, e.Target, newChild, output,
        collapsedNodes, pts, ped, collapsedArray);
      }
    collapsedArray->InsertValue(newChild, 0);
    }
  else
    {
    collapsedArray->InsertValue(newChild, 1);
    }
}

int vtkTreeCollapseFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  // Storing the inputTree and outputTree handles
  vtkTree* inputTree = vtkTree::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkTree* outputTree = vtkTree::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (inputTree->GetNumberOfVertices() == 0)
    {
    return 1;
    }

  vtkSmartPointer<vtkMutableDirectedGraph> builder =
    vtkSmartPointer<vtkMutableDirectedGraph>::New();

  // Copy collapsed nodes to the set
  vtksys_stl::set<vtkIdType> collapsedNodes;
  vtkIdType cc;
  for ( cc = 0; cc < this->CollapsedNodes->GetNumberOfTuples(); ++ cc )
    {
    collapsedNodes.insert(this->CollapsedNodes->GetValue(cc));
    }

  builder->GetVertexData()->CopyAllocate(inputTree->GetVertexData());
  builder->GetEdgeData()->CopyAllocate(inputTree->GetEdgeData());
  vtkCharArray* collapsedArray = vtkCharArray::New();
  collapsedArray->SetName("Collapsed");
  builder->GetVertexData()->AddArray(collapsedArray);
  collapsedArray->Delete();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkIdTypeArray* ped = vtkIdTypeArray::SafeDownCast(
    inputTree->GetVertexData()->GetAbstractArray("PedigreeVertexId"));
  vtkTreeCollapseFilterCollapsedDFS(inputTree, 0, -1, builder, collapsedNodes, points,
    ped, collapsedArray);
  builder->SetPoints(points);
  builder->Squeeze();

  outputTree->CheckedShallowCopy(builder);

  return 1;
}

void vtkTreeCollapseFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << *this->CollapsedNodes << endl;
}
