// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#ifndef CellLineage_H
#define CellLineage_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QString>

#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
#include "vtkStdString.h"

#include <map>

// Forward Qt class declarations
class Ui_CellLineage;
class vtkObject;
class vtkQtTreeView;
class vtkTreeToQtModelAdapter;

// Forward VTK class declarations
class vtkAnnotationLink;
class vtkCommand;
class vtkDataRepresentation;
class vtkEventQtSlotConnect;
class vtkGraph;
class vtkLineageView;
class vtkTable;
class vtkTreeReader;

// The view updater
class CellLineageUpdater;

class CellLineage : public QMainWindow
{
  Q_OBJECT

public:

  // Constructor/Destructor
  CellLineage( QWidget * iParent = 0, Qt::WindowFlags iFlags = 0 );
  ~CellLineage();

public slots:

  // Application slots
  virtual void slotOpenLineageData();
  virtual void slotExit();

  // Description:
  // Toggle the mouse mode between selection and collapsing/expanding
  void slotSetCollapseMode(int on);

  // Description:
  // Set the labels on/off
  void slotSetLabels(int on);

  // Description:
  // Set whether to use radial layout.
  void slotSetRadialLayout(int radial);

  // Description:
  // Set the radial layout angle.
  void slotSetRadialAngle(int angle);

  // Description:
  // Set the log spacing for the layout.
  void slotSetLogSpacingFactor(double spacing);

  // Description:
  // Set whether to see the back plane
  void slotSetBackPlane(int state);

  // Description:
  // Set whether to see the iso contour
  void slotSetIsoContour(int state);

  // Description:
  // Set when elbow is turned on
  void slotSetElbow(int state);

  // Description:
  // Set the elbow angle
  void slotSetElbowAngle(int value);

  void slotEnableScale(int state);
  void slotChangeScale(QString array);

  void slotEnableColorCode(int state);
  void slotChangeColorCode(QString array);

  void slotChangeLabel(QString array);

protected:

protected slots:

  // Description:
  // Set the global time value for just the tree layout
  void slotGlobalTimeValueChanging(int value);

  // Description:
  // Set the global time value for all views
  void slotSetGlobalTimeValue(int value);

  // Description:
  // Called when selection changed in the Qt tree view
  void slotSelectionChanged();

private:

  // Methods

  // Description: Browse for and read the Lineage data
  int readLineageData();

  // Description: Set up the Lineage list view of the data
  void setUpLineageListView();

  // Members
  vtkTreeReader*           LineageReader;
  vtkLineageView*          LineageView;
  vtkDataRepresentation*   LineageViewRep;
  vtkQtTreeView*           QtTreeView;
  vtkAnnotationLink*       AnnotationLink;
  CellLineageUpdater* Updater;
  vtkEventQtSlotConnect* Connect;

  // Designer form
  Ui_CellLineage *ui;
};

#endif // CellLineage_H
