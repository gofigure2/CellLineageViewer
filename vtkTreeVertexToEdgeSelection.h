// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// .NAME vtkTreeVertexToEdgeSelection - Converts tree vertex selection to edge
// selection.
//
// .SECTION Description

#ifndef __vtkTreeVertexToEdgeSelection_h
#define __vtkTreeVertexToEdgeSelection_h

#include "vtkSelectionAlgorithm.h"

class vtkTree;
class vtkTreeVertexToEdgeSelectionInternals;

class vtkTreeVertexToEdgeSelection : public vtkSelectionAlgorithm
{
public:
  static vtkTreeVertexToEdgeSelection* New();
  vtkTypeRevisionMacro(vtkTreeVertexToEdgeSelection, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkTreeVertexToEdgeSelection();
  ~vtkTreeVertexToEdgeSelection();

  virtual int FillInputPortInformation(
    int port, vtkInformation* info);

  int RequestData(
    vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

  vtkTreeVertexToEdgeSelectionInternals* Internals;
  vtkTree* Tree;
  unsigned long TreeMTime;
    
private:
  vtkTreeVertexToEdgeSelection(const vtkTreeVertexToEdgeSelection&); // Not implemented
  void operator=(const vtkTreeVertexToEdgeSelection&);   // Not implemented
};

#endif

