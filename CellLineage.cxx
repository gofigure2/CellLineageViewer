// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include <QApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QListView>
#include <QPushButton>
#include <QProgressBar>
#include <QString>
#include <QTimer>
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
#include <vtkVolumeViewer.h>
#include <vtkXMLImageDataReader.h>

#include <vector>

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

// Constructor
CellLineage::
CellLineage( QWidget* iParent, Qt::WindowFlags iFlags ) :
  QMainWindow( iParent, iFlags )
{
  this->ui = new Ui_CellLineage;
  this->ui->setupUi(this);

  this->LineageReader       = vtkTreeReader::New();
  this->LineageView         = vtkLineageView::New();
  this->VolumeReader        = vtkXMLImageDataReader::New();
  this->VolumeView          = vtkVolumeViewer::New();
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

  this->GeneTable = 0;
  this->GeneGraph = 0;

  // Lineage Viewer needs to get my render window
  this->LineageView->SetInteractor(this->ui->vtkLineageViewWidget->GetInteractor());
  this->ui->vtkLineageViewWidget->SetRenderWindow(this->LineageView->GetRenderWindow());

  // Volume Viewer needs to get my render window
  this->VolumeView->SetRenderWindow(this->ui->vtkVolumeViewWidget->GetRenderWindow());

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
  connect(this->ui->distanceCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetDistanceByTime(int)));
  connect(this->ui->colorEdgesCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(slotSetColorEdges(int)));
  slotSetDistanceByTime(1);

  // Time controls
  this->globalTime = 1; // Start time at 1 :)
  this->ui->timeSlider->setMinimum(0);
  this->ui->timeSlider->setMaximum(37+3*99);
  connect(this->ui->timeSlider, SIGNAL(valueChanged(int)),
    this, SLOT(slotSetGlobalTimeValue(int)));
  connect(this->ui->timeSlider, SIGNAL(sliderMoved(int)),
    this, SLOT(slotGlobalTimeValueChanging(int)));
  connect(this->ui->vcr, SIGNAL(play()), this, SLOT(slotVCRPlay()));
  connect(this->ui->vcr, SIGNAL(pause()), this, SLOT(slotVCRPause()));
  connect(this->ui->vcr, SIGNAL(back()), this, SLOT(slotVCRBack()));
  connect(this->ui->vcr, SIGNAL(forward()), this, SLOT(slotVCRForward()));
  connect(this->ui->vcr, SIGNAL(first()), this, SLOT(slotVCRFirst()));
  connect(this->ui->vcr, SIGNAL(last()), this, SLOT(slotVCRLast()));

  // Application signals and slots
  connect(this->ui->actionOpenLineageFile, SIGNAL(triggered()), this, SLOT(slotOpenLineageData()));
  connect(this->ui->actionOpenGeneData, SIGNAL(triggered()), this, SLOT(slotOpenGeneData()));
  connect(this->ui->actionOpenDataFile, SIGNAL(triggered()), this, SLOT(slotOpenVolumeData()));
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

  this->SelectingGenesFromCells = false;
  this->SelectingCellsFromGenes = false;
}

CellLineage::~CellLineage()
{
  this->LineageReader->Delete();
  this->LineageView->Delete();
  this->VolumeReader->Delete();
  this->VolumeView->Delete();
  this->QtTreeView->Delete();
  this->AnnotationLink->Delete();
  this->Updater->Delete();
  this->Connect->Delete();

  if (this->GeneTable != NULL)
    {
    this->GeneTable->Delete();
    this->GeneTable = NULL;
    }
  if (this->GeneGraph != NULL)
    {
    this->GeneGraph->Delete();
    this->GeneGraph = NULL;
    }
}

// Description:
// Set the global time value for all views
void CellLineage::slotGlobalTimeValueChanging(int value)
{
  this->globalTime = value;

  // Set the value of the slider and line edit.
  this->ui->timeSlider->setValue(value);

  // Have the lineage view update itself
  this->LineageView->SetCurrentTime(value);
  this->LineageView->Render();
}

// Description:
// Set the global time value for all views
void CellLineage::slotSetGlobalTimeValue(int value)
{
  this->globalTime = value;

  // Put mouse into busy mode
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Read in volume data
  this->readVolumeDataTimeStep(this->globalTime);

  // Set the value of the slider and line edit.
  this->ui->timeSlider->setValue(value);

  // Have the volume view update itself
  this->VolumeView->UpdateView();

  // Have the lineage view update itself
  this->LineageView->SetCurrentTime(value);
  this->LineageView->Render();

  // Have default cursor come back
  QApplication::restoreOverrideCursor();
}

// Description:
// VCR Slots
void CellLineage::slotVCRPlay()
{
  this->globalTime++;
  slotSetGlobalTimeValue(this->globalTime);
}
void CellLineage::slotVCRPause()
{
  this->globalTime;
  slotSetGlobalTimeValue(this->globalTime);
}
void CellLineage::slotVCRBack()
{
  this->globalTime--;
  slotSetGlobalTimeValue(this->globalTime);
}
void CellLineage::slotVCRForward()
{
  this->globalTime++;
  slotSetGlobalTimeValue(this->globalTime);
}
void CellLineage::slotVCRFirst()
{
  this->globalTime = 1;
  slotSetGlobalTimeValue(this->globalTime);
}
void CellLineage::slotVCRLast()
{
  this->globalTime = 99;
  slotSetGlobalTimeValue(this->globalTime);
}

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

// Description:
// Set whether to color edges by a scalar.
void CellLineage::slotSetColorEdges(int value)
{
  this->LineageView->SetEdgeScalarVisibility(value);
  this->LineageView->Render();
}

// Description:
// Set the radial layout angle.
void CellLineage::slotSetRadialAngle(int angle)
{
  this->LineageView->SetRadialAngle(angle);
  this->LineageView->Render();
}

// Description:
// Set the log spacing for the layout.
void CellLineage::slotSetLogSpacingFactor(double spacing)
{
  this->LineageView->SetLogSpacingFactor(spacing);
  this->LineageView->Render();
}

// Description:
// Set whether to set the back plane
void CellLineage::slotSetBackPlane(int state)
{
  this->LineageView->SetBackPlane(state);
  this->LineageView->Render();
}


// Description:
// Set whether to set the iso contour
void CellLineage::slotSetIsoContour(int state)
{
  this->LineageView->SetIsoContour(state);
  this->LineageView->Render();
}

// Action to be taken upon gene expression file open
void CellLineage::slotOpenGeneData()
{
  QString fileName = QFileDialog::getOpenFileName(
    this,
    "Select the gene expression data file",
    QDir::homePath(),
    "Comma Separated Values Files (*.csv);;All Files (*.*)");

  if (fileName.isNull())
    {
    return;
    }

  // Read in the gene table
  vtkDelimitedTextReader* reader = vtkDelimitedTextReader::New();
  reader->SetFileName(fileName.toStdString().c_str());
  reader->SetMergeConsecutiveDelimiters(false);
  reader->SetHaveHeaders(true);
  reader->Update();
  this->GeneTable = reader->GetOutput();
  this->GeneTable->Register(0);
  reader->Delete();

  // Make a graph of the gene table
  vtkTableToGraph* tableToGraph = vtkTableToGraph::New();
  tableToGraph->SetInput(this->GeneTable);
  tableToGraph->AddLinkVertex("cell", "cell");
  tableToGraph->AddLinkVertex("gene", "gene");
  tableToGraph->AddLinkEdge("cell", "gene");
  tableToGraph->Update();
  this->GeneGraph = tableToGraph->GetOutput();
  this->GeneGraph->Register(0);
  tableToGraph->Delete();

  // Make mapping from table ids to graph vertices and back.
  // Make mapping from tree ids to graph vertices and back.
  vtkStringArray* domainCol = vtkStringArray::SafeDownCast(
    this->GeneGraph->GetVertexData()->GetAbstractArray("domain"));
  vtkStringArray* valueCol = vtkStringArray::SafeDownCast(
    this->GeneGraph->GetVertexData()->GetAbstractArray("label"));
  for (vtkIdType v = 0; v < this->GeneGraph->GetNumberOfVertices(); v++)
    {
    vtkStdString d = domainCol->GetValue(v);
    vtkStdString val = valueCol->GetValue(v);
    if (d == "cell")
      {
      this->CellToVertex[val] = v;
      }
    else
      {
      this->GeneToVertex[val] = v;
      }
    }

  // Make a list of gene names
  // Make GeneToTableRow map
  std::map<vtkStdString, vtkIdType>::iterator it, itEnd;
  it = this->GeneToVertex.begin();
  itEnd = this->GeneToVertex.end();
  QList<QStandardItem*> genes;
  for (vtkIdType r = 0; it != itEnd; ++it, ++r)
    {
    this->GeneToTableRow[it->first] = r;
    genes.append(new QStandardItem(QString(it->first.c_str())));
    }
  QStandardItemModel* model = new QStandardItemModel;
  model->appendColumn(genes);
  model->setHeaderData(0, Qt::Horizontal, QVariant("Gene"));
  this->ui->geneTableView->setModel(model);

  // Make CellToTreePedigree map
  vtkTree* tree = vtkTree::SafeDownCast(
    this->QtTreeView->GetRepresentation()->GetInputConnection()->
      GetProducer()->GetOutputDataObject(0));
  vtkStringArray* nameArr = vtkStringArray::SafeDownCast(
    tree->GetVertexData()->GetAbstractArray("name"));
  for (vtkIdType i = 0; i < nameArr->GetNumberOfTuples(); ++i)
    {
    this->CellToTreePedigree[nameArr->GetValue(i)] = i;
    }

  // Connect signals to slots
  connect(
    this->ui->geneTableView->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(slotGeneSelectionChanged()));
  this->Connect->Connect(
    this->QtTreeView->GetRepresentation(), vtkCommand::SelectionChangedEvent,
    this, SLOT(slotSelectGenesFromCells(vtkObject*, unsigned long, void*, void*)));
  this->Connect->Connect(
    this->LineageView->GetRepresentation(), vtkCommand::SelectionChangedEvent,
    this, SLOT(slotSelectGenesFromCells(vtkObject*, unsigned long, void*, void*)));
}

void CellLineage::slotGeneSelectionChanged()
{
  if (!this->SelectingGenesFromCells)
    {
    // Convert the gene selection into a cell selection.
    QItemSelection cellSelection;
    QModelIndexList geneList = this->ui->geneTableView->selectionModel()->selectedRows();
    //vtkTreeToQtModelAdapter* model = qobject_cast<vtkTreeToQtModelAdapter*>(this->ui->treeTextView->model());

    vtkSmartPointer<vtkSelection> selection =
      vtkSmartPointer<vtkSelection>::New();
    vtkSmartPointer<vtkSelectionNode> selectionNode =
      vtkSmartPointer<vtkSelectionNode>::New();
    vtkSmartPointer<vtkStringArray> selectionArr =
      vtkSmartPointer<vtkStringArray>::New();
    selectionArr->SetName("name");
    selectionNode->SetSelectionList(selectionArr);
    selectionNode->SetContentType(vtkSelectionNode::VALUES);
    selectionNode->SetFieldType(vtkSelectionNode::VERTEX);
    selection->AddNode(selectionNode);

    vtkStringArray* valueArray = vtkStringArray::SafeDownCast(
      this->GeneGraph->GetVertexData()->GetAbstractArray("label"));
    vtkSmartPointer<vtkOutEdgeIterator> it =
      vtkSmartPointer<vtkOutEdgeIterator>::New();
    QAbstractItemModel* tableModel = this->ui->geneTableView->model();
    for (int g = 0; g < geneList.size(); g++)
      {
      QString gene = tableModel->data(geneList[g]).toString();
      vtkIdType geneVertex = this->GeneToVertex[gene.toStdString()];

      // Get adjacent graph edges
      this->GeneGraph->GetOutEdges(geneVertex, it);
      while (it->HasNext())
        {
        vtkOutEdgeType e = it->Next();
        vtkIdType cellVertex = e.Target;
        vtkStdString cellName = valueArray->GetValue(cellVertex);
        vtkIdType pedigree = this->CellToTreePedigree[cellName];
        selectionArr->InsertNextValue(cellName);
        //cerr << "selecting cell (" << cellVertex << "," << cellName << "," << pedigree << ") from gene: (" << gene.toStdString() << "," << geneVertex << ")" << endl;
        //QModelIndex cellIndex = model->PedigreeToQModelIndex(pedigree);
        //cellSelection.select(cellIndex, cellIndex);
        }
      }
    //this->SelectingCellsFromGenes = true;
    vtkSmartPointer<vtkSelection> converted;
    converted.TakeReference(vtkConvertSelection::ToIndexSelection(selection, this->LineageReader->GetOutput()));
    this->AnnotationLink->SetCurrentSelection(converted);
    this->QtTreeView->Update();
    this->LineageView->Render();
    //this->ui->treeTextView->selectionModel()->select(cellSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    //this->SelectingCellsFromGenes = false;
    }
}

void CellLineage::slotSelectGenesFromCells(vtkObject*, unsigned long, void*, void* callData)
{
  vtkSelection* selection = reinterpret_cast<vtkSelection*>(callData);
  vtkSelectionNode* node = 0;
  vtkIdTypeArray* arr = 0;
  if (selection)
    {
    node = selection->GetNode(0);
    }
  if (node)
    {
    arr = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
    }
  if (!this->SelectingCellsFromGenes && arr)
    {
    QAbstractItemModel* tableModel = this->ui->geneTableView->model();
    QItemSelection geneSelection;
    vtkStringArray* valueArray = vtkStringArray::SafeDownCast(
      this->GeneGraph->GetVertexData()->GetAbstractArray("label"));
    vtkSmartPointer<vtkOutEdgeIterator> it =
      vtkSmartPointer<vtkOutEdgeIterator>::New();
    vtkTree* tree = vtkTree::SafeDownCast(
      this->QtTreeView->GetRepresentation()->GetInputConnection()->
        GetProducer()->GetOutputDataObject(0));
    vtkStringArray* nameArray = vtkStringArray::SafeDownCast(
      tree->GetVertexData()->GetAbstractArray("name"));
    for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); i++)
      {
      if (arr->GetValue(i) >= nameArray->GetNumberOfTuples())
        {
        continue;
        }

      vtkStdString cell = nameArray->GetValue(arr->GetValue(i));
      vtkIdType cellVertex = this->CellToVertex[cell];

      // Get adjacent graph edges
      this->GeneGraph->GetOutEdges(cellVertex, it);
      while (it->HasNext())
        {
        vtkOutEdgeType e = it->Next();
        vtkIdType geneVertex = e.Target;
        vtkStdString geneName = valueArray->GetValue(geneVertex);
        vtkIdType row = this->GeneToTableRow[geneName];
        QModelIndex geneIndex = tableModel->index(row, 0);
        geneSelection.select(geneIndex, geneIndex);
        }
      }
    this->SelectingGenesFromCells = true;
    this->ui->geneTableView->selectionModel()->select(geneSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    this->SelectingGenesFromCells = false;
    }
}

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
  //LineageReader->Get
  this->ui->scaleType->clear();
  this->ui->scaleType->addItem("scale item");

  this->ui->colorCodeType->clear();
  this->ui->colorCodeType->addItem("color item");
  // init color code / active scalar
  // init scale
}

void CellLineage::slotSetDistanceByTime(int state)
{
  this->LineageView->SetDistanceArrayName(state ? "XPos" : NULL);
  this->LineageView->Render();
}

void CellLineage::slotSetElbow(int state)
{
  this->LineageView->SetElbow(state?1:0);
  this->LineageView->Render();
}

void CellLineage::slotSetElbowAngle(int value)
{
  this->LineageView->SetElbowAngle(static_cast<double>(value)/100.0);
  this->LineageView->Render();
}

void CellLineage::slotSelectionChanged()
{
  this->LineageView->Render();
}

// Set up the lineage list view of the data
void CellLineage::setUpLineageListView()
{
  // Expand all
  this->QtTreeView->ExpandAll();

  // Now resize the first column to fit it's contents
  this->QtTreeView->ResizeColumnToContents(0);
}

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

// Action to be taken upon volume file open
void CellLineage::slotOpenVolumeData()
{
  // Browse for and read the lineage data
  if (this->readVolumeData())
    {
    return;
    }

  // Set up the volume view of this data
  this->VolumeView->SetInputConnection(this->VolumeReader->GetOutputPort(0));
}

// Browse for and read in the volume data
int CellLineage::readVolumeData()
{

  // Open the lineage data file
  QString fileName = QFileDialog::getOpenFileName(
    this,
    "Select the first volume time series file",
    QDir::homePath(),
    "Volume Data (*.vti);;All Files (*.*)");

  if (fileName.isNull())
    {
    return -1;
    }

  // Grab the data directory for the volume data
  QFileInfo info(fileName);
  this->volumeDataDir = info.path();

  // Create volume reader
  this->VolumeReader->SetFileName( fileName.toAscii() );
  this->VolumeReader->Update();
  return 0;
}

// Description: Open a specific timestep (do not browse)
int CellLineage::readVolumeDataTimeStep(int timeStep)
{
  int volumeTime = (timeStep - 37)/3;
  if (volumeTime < 0) volumeTime = 0;

  // Hack alert this method makes lots of hardcode assumptions
  QString file_pattern;
  file_pattern.sprintf("%s/cache%d.vti", this->volumeDataDir.toAscii().data(), volumeTime);

  // Read in this particular file (time step)
  this->VolumeReader->SetFileName( file_pattern.toAscii() );
  this->VolumeReader->Update();

  // Success I guess
  return 0;
}

void CellLineage::slotExit() {
  qApp->exit();
}
