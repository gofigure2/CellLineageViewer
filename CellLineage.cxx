// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include <QApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QString>
#include <QStandardItem>
#include <QStandardItemModel>

#include "ui_CellLineage.h"
#include "CellLineage.h"

#include <vtkAlgorithmOutput.h>
#include <vtkAnnotationLink.h>
#include <vtkConvertSelection.h>
#include <vtkDataRepresentation.h>
#include <vtkDelimitedTextReader.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkGraph.h>
#include <vtkIdTypeArray.h>
#include <vtkLineageView.h>
#include <vtkOutEdgeIterator.h>
#include <vtkPointData.h>
#include <vtkQtTreeView.h>
#include <vtkSelectionNode.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTableToGraph.h>
#include <vtkTableWriter.h>
#include <vtkTree.h>
#include <vtkTreeReader.h>
#include <vtkQtTreeModelAdapter.h>
#include <vtkVariant.h>
#include <vtkViewTheme.h>

#include <vector>

//----------------------------------------------------------------------------
class CellLineageUpdater : public vtkCommand
{
public:
  static CellLineageUpdater* New()
  { return new CellLineageUpdater; }

  void AddView(vtkView* view)
  {
    this->Views.push_back(view);
    view->AddObserver(vtkCommand::SelectionChangedEvent, this);
  }

  virtual void Execute(vtkObject* , unsigned long , void*)
  {
    std::vector< vtkView* >::iterator it = Views.begin();
    std::vector< vtkView* >::iterator end = Views.end();

    while( it != end )
      {
      (*it)->Update();
      ++it;
      }
  }
private:
  CellLineageUpdater() { }
  ~CellLineageUpdater() { }
  std::vector<vtkView*> Views;
};
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constructor
CellLineage::
CellLineage( QWidget* iParent, Qt::WindowFlags iFlags ) :
  QMainWindow( iParent, iFlags )
{
  this->ui = new Ui_CellLineage;
  this->ui->setupUi(this);

  this->LineageReader       = vtkTreeReader::New();
  this->LineageView         = vtkLineageView::New();
  this->QtTreeView          = vtkQtTreeView::New();
  this->AnnotationLink      = vtkAnnotationLink::New();
  this->Updater             = CellLineageUpdater::New();
  this->Connect             = vtkEventQtSlotConnect::New();

  vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
  theme->SetBackgroundColor(1.0, 1.0, 1.0);
  theme->SetBackgroundColor2(1.0, 1.0, 1.0);
  this->LineageView->ApplyViewTheme(theme);

  this->Updater->AddView(this->LineageView);
  this->Updater->AddView(this->QtTreeView);

  this->ui->treeTextView->layout()->addWidget(this->QtTreeView->GetWidget());

  // Lineage Viewer needs to get my render window
  this->LineageView->SetInteractor(this->ui->vtkLineageViewWidget->GetInteractor());
  this->ui->vtkLineageViewWidget->SetRenderWindow(this->LineageView->GetRenderWindow());

  // Lineage view parameters
  connect(this->ui->collapseModeCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetCollapseMode(int)));
  connect(this->ui->labelsCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetLabels(int)));
  connect(this->ui->radialLayoutCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetRadialLayout(int)));
  connect(this->ui->radialLayoutAngleSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(slotSetRadialAngle(int)));
  connect(this->ui->logSpacingSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(slotSetLogSpacingFactor(double)));
  connect(this->ui->backPlaneCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetBackPlane(int)));
  connect(this->ui->isoContourCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetIsoContour(int)));
  connect(this->ui->elbowCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetElbow(int)));
  connect(this->ui->elbowAngleSlider, SIGNAL(valueChanged(int)),
    this, SLOT(slotSetElbowAngle(int)));
  // new slots
  //for scaling - should turn it on by default?
  connect(this->ui->scaleBy, SIGNAL(stateChanged(int)),
    this, SLOT(slotEnableScale(int)));
  connect(this->ui->scaleType, SIGNAL(currentIndexChanged(QString)),
    this, SLOT(slotChangeScale(QString)));
  // color coding
  connect(this->ui->colorCodeBy, SIGNAL(stateChanged(int)),
    this, SLOT(slotEnableColorCode(int)));
  connect(this->ui->colorCodeType, SIGNAL(currentIndexChanged(QString)),
    this, SLOT(slotChangeColorCode(QString)));
  // labels
  connect(this->ui->labelType, SIGNAL(currentIndexChanged(QString)),
    this, SLOT(slotChangeLabel(QString)));
  // Time controls
  this->ui->timeSlider->setMinimum(0);
  this->ui->timeSlider->setMaximum(100);
  connect(this->ui->timeSlider, SIGNAL(valueChanged(int)),
    this, SLOT(slotSetGlobalTimeValue(int)));
  connect(this->ui->timeSlider, SIGNAL(sliderMoved(int)),
    this, SLOT(slotGlobalTimeValueChanging(int)));

  // Application signals and slots
  connect(this->ui->actionOpenLineageFile, SIGNAL(triggered()), this, SLOT(slotOpenLineageData()));
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
CellLineage::~CellLineage()
{
  this->LineageReader->Delete();
  this->LineageView->Delete();
  this->QtTreeView->Delete();
  this->AnnotationLink->Delete();
  this->Updater->Delete();
  this->Connect->Delete();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set the global time value for all views
void CellLineage::slotGlobalTimeValueChanging(int value)
{
  // Set the value of the slider and line edit.
  this->ui->timeSlider->setValue(value);

  // Have the lineage view update itself
  this->LineageView->SetCurrentTime(value);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set the global time value for all views
void CellLineage::slotSetGlobalTimeValue(int value)
{
  // Set the value of the slider and line edit.
  this->ui->timeSlider->setValue(value);

  // Have the lineage view update itself
  this->LineageView->SetCurrentTime(value);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set mouse mode
void CellLineage::slotSetCollapseMode(int on)
{
  if (on)
    {
    this->LineageView->SetSelectMode(vtkLineageView::COLLAPSE_MODE);
    }
  else
    {
    this->LineageView->SetSelectMode(vtkLineageView::SELECT_MODE);
    }
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set labels on/off
void CellLineage::slotSetLabels(int on)
{
  if (on)
    {
    this->LineageView->SetLabelsOn();
    }
  else
    {
    this->LineageView->SetLabelsOff();
    }
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set whether to use radial layout.
void CellLineage::slotSetRadialLayout(int radial)
{
  if ( !radial )
    {
    if ( this->LineageView->GetRadialAngle() > 179 )
      {
      this->LineageView->BlockUpdateOn();
      this->ui->radialLayoutAngleSpinBox->setValue(120);
      this->LineageView->BlockUpdateOff();
      }
    this->ui->radialLayoutAngleSpinBox->setMinimum(30);
    this->ui->radialLayoutAngleSpinBox->setMaximum(179);
    }
  else
    {
    this->ui->radialLayoutAngleSpinBox->setMinimum(90);
    this->ui->radialLayoutAngleSpinBox->setMaximum(360);
    }
  this->LineageView->SetRadialLayout(radial);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set the radial layout angle.
void CellLineage::slotSetRadialAngle(int angle)
{
  this->LineageView->SetRadialAngle(angle);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set the log spacing for the layout.
void CellLineage::slotSetLogSpacingFactor(double spacing)
{
  this->LineageView->SetLogSpacingFactor(spacing);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set whether to set the back plane
void CellLineage::slotSetBackPlane(int state)
{
  this->LineageView->SetBackPlane(state);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Description:
// Set whether to set the iso contour
void CellLineage::slotSetIsoContour(int state)
{
  this->LineageView->SetIsoContour(state);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Action to be taken upon lineage file open
void CellLineage::slotOpenLineageData()
{
  // Browse for and read the lineage data
  if (this->readLineageData())
    {
    return;
    }

  // Set up the lineage view of this data
  this->LineageView->AddRepresentationFromInputConnection(
    this->LineageReader->GetOutputPort());
  this->QtTreeView->AddRepresentationFromInputConnection(
    this->LineageReader->GetOutputPort());
  this->QtTreeView->Update();

  // Set up the text view of the lineage data
  this->setUpLineageListView();

  this->LineageView->GetRepresentation()->SetAnnotationLink(this->AnnotationLink);
  this->QtTreeView->GetRepresentation()->SetAnnotationLink(this->AnnotationLink);

  // Link Qt tree selection to the linkage viewer selection
  this->Connect->Connect(this->QtTreeView->GetRepresentation(),
    vtkCommand::SelectionChangedEvent,
    this, SLOT(slotSelectionChanged()));

  // Update combo boxes (fill content with arrays names)
  // how many fields do we have?
  int numberOfArrays = LineageReader->GetOutput()->GetVertexData()->GetNumberOfArrays();
  this->ui->scaleType->clear();
  this->ui->colorCodeType->clear();
  this->ui->labelType->clear();

  // fill comboxes according to the data
  for(int i=0;i<numberOfArrays; i++)
    {
    const char* name =
        LineageReader->GetOutput()->GetVertexData()->GetArrayName(i);
    // if data array (i.e. numbers), add it
    if(LineageReader->GetOutput()->GetVertexData()->GetArray(name))
      {
      this->ui->scaleType->addItem(name);
      this->ui->colorCodeType->addItem(name);
      this->ui->labelType->addItem(name);
      }
    }

  // set the active scalar, update mappers and LUTs
  char* activeScalar = this->ui->colorCodeType->currentText().toLocal8Bit().data();

  //vertex: node (small square)
  this->LineageView->SetVertexColorFieldName(activeScalar);
  this->LineageView->SetEdgeColorFieldName(activeScalar);

  // update time slider for the iso contours
  double* range =
      LineageReader->GetOutput()->GetVertexData()->GetArray(activeScalar)->GetRange();
  this->ui->timeSlider->setMinimum(range[0]);
  this->ui->timeSlider->setMaximum(range[1]);
  this->ui->timeSlider->setValue(range[0]);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotEnableScale(int state)
{
  //scale
  this->LineageView->SetDistanceArrayName
  (state ? this->ui->scaleType->currentText().toLocal8Bit().data() : NULL);

  //update visu
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotChangeScale(QString array)
{
  //scale
  this->LineageView->SetDistanceArrayName
  (this->ui->scaleBy->isChecked() ? array.toLocal8Bit().data() : NULL);

  //update visu
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotEnableColorCode(int state)
{
  // update visibility
  this->LineageView->SetEdgeScalarVisibility(state);

  //update visu
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotChangeColorCode(QString array)
{
 this->LineageView->SetVertexColorFieldName(array.toLocal8Bit().data());
 this->LineageView->SetEdgeColorFieldName(array.toLocal8Bit().data());

  // update time slider for the iso contours
  double* range =
      LineageReader->GetOutput()->GetVertexData()->GetArray(array.toLocal8Bit().data())->GetRange();
  this->ui->timeSlider->setMinimum(range[0]);
  this->ui->timeSlider->setMaximum(range[1]);
  this->ui->timeSlider->setValue(range[0]);

  //update visu
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotChangeLabel(QString array)
{
  //this->LineageView->SetLabelFieldName(array.toLocal8Bit().data());

  //update visu
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotSetElbow(int state)
{
  this->LineageView->SetElbow(state?1:0);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotSetElbowAngle(int value)
{
  this->LineageView->SetElbowAngle(static_cast<double>(value)/100.0);
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotSelectionChanged()
{
  this->LineageView->Render();
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Set up the lineage list view of the data
void CellLineage::setUpLineageListView()
{
  // Expand all
  this->QtTreeView->ExpandAll();

  // Now resize the first column to fit it's contents
  this->QtTreeView->ResizeColumnToContents(0);
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Browse for and read in the lineage data
int CellLineage::readLineageData()
{
  QDir dir;

  // Open the lineage data file
  QString fileName = QFileDialog::getOpenFileName(
    this,
    "Select the lineage tree file",
    QDir::homePath(),
    "VTK Tree Files (*.vtk);;All Files (*.*)");

  if (fileName.isNull())
    {
    return -1;
    }

  // Create lineage reader
  this->LineageReader->SetFileName( fileName.toAscii() );
  this->LineageReader->Update();
  return 0;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CellLineage::slotExit() {
  qApp->exit();
}
