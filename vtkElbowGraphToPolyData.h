// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// .NAME vtkElbowGraphToPolyData - convert a vtkGraph to vtkPolyData
//
// .SECTION Description
// Subclass that converts a vtkGraph to a vtkPolyData.  This assumes that the
// points of the graph have already been filled (perhaps by vtkGraphLayout),
// and coverts all the edge of the graph into set of lines with elbow.  The
// vertex data is passed along to the point data, and the edge data is passed
// along to the cell data.
//
// Only the owned graph edges (i.e. edges with ghost level 0) are copied
// into the vtkPolyData.

#ifndef __vtkElbowGraphToPolyData_h
#define __vtkElbowGraphToPolyData_h

#include "vtkGraphToPolyData.h"

class vtkElbowGraphToPolyDataInternal;

class vtkElbowGraphToPolyData : public vtkGraphToPolyData
{
public:
  static vtkElbowGraphToPolyData *New();
  vtkTypeRevisionMacro(vtkElbowGraphToPolyData,vtkGraphToPolyData);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the factor of angle for the elbow
  vtkSetClampMacro(Factor, double, -1, 1);
  vtkGetMacro(Factor, double);

  // Description:
  // Turn elbowing on or off
  vtkSetClampMacro(Elbow, int, 0, 1);
  vtkBooleanMacro(Elbow, int);
  vtkGetMacro(Elbow, int);

protected:
  vtkElbowGraphToPolyData();
  ~vtkElbowGraphToPolyData();

  // Description:
  // Convert the vtkGraph into vtkPolyData.
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkElbowGraphToPolyDataInternal* Internals;

  double Factor;
  int Elbow;

private:
  vtkElbowGraphToPolyData(const vtkElbowGraphToPolyData&);  // Not implemented.
  void operator=(const vtkElbowGraphToPolyData&);  // Not implemented.
};

#endif

