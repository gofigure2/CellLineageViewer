// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

// QT includes
#include <QApplication>
#include <QCleanlooksStyle>
#include "CellLineage.h"

#include "vtkFileOutputWindow.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkTable.h"
#include "vtkTree.h"
#include "vtkTreeReader.h"
#include "vtkTreeWriter.h"

int main( int argc, char** argv )
{
  // QT Stuff
  QApplication app( argc, argv );
  //QApplication::setStyle(new QCleanlooksStyle);

  CellLineage myCellLineage;
  myCellLineage.show();

  int ret = app.exec();
  return ret;
}
