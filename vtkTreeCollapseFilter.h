// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// .NAME vtkTreeCollapseFilter - collapses specified subtrees in the tree
//
// .SECTION Description
// This filter takes input list of nodes and collapses (hides) the subtrees
// rooted at those nodes.

#ifndef __vtkTreeCollapseFilter_h
#define __vtkTreeCollapseFilter_h

#include "vtkTreeAlgorithm.h"

class vtkTreeCollapseFilter : public vtkTreeAlgorithm
{
public:
  static vtkTreeCollapseFilter *New();
  vtkTypeRevisionMacro(vtkTreeCollapseFilter,vtkTreeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set and get the list of nodes that will be collapsed.
  virtual void SetCollapsedNodes(vtkIdTypeArray* ids);
  vtkGetObjectMacro(CollapsedNodes, vtkIdTypeArray);

protected:
  vtkTreeCollapseFilter();
  ~vtkTreeCollapseFilter();

  // Description:
  // Convert the vtkGraph into vtkPolyData.
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkIdTypeArray* CollapsedNodes;

private:
  vtkTreeCollapseFilter(const vtkTreeCollapseFilter&);  // Not implemented.
  void operator=(const vtkTreeCollapseFilter&);  // Not implemented.
};

#endif

