// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "vtkAbstractArray.h"
#include "vtkDelimitedTextReader.h"
#include "vtkSmartPointer.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkVariant.h"

// Converts file of the form
// Cell Name,gene1,gene2,gene3,...,geneN
// cell1,1,0,0,...,0
// cell2,0,0,1,...,0
// cell3,0,0,0,...,0
// ...
// cellM,0,0,0,...,1
//
// To the form
// cell,gene
// cell1,gene1
// cell2,gene3
// ...
// cellM,geneN

int main( int argc, char** argv )
{
  if (argc != 2)
    {
    cerr << "Usage: TableToAdjacencyList outTable.csv > adjTable.csv" << endl;
    return 1;
    }
  vtkSmartPointer<vtkDelimitedTextReader> tr =
    vtkSmartPointer<vtkDelimitedTextReader>::New();
  tr->SetHaveHeaders(true);
  tr->SetMergeConsecutiveDelimiters(false);
  tr->SetFieldDelimiterCharacters(",");
  tr->SetFileName(argv[1]);
  tr->Update();
  vtkTable* tab = tr->GetOutput();
  cout << "cell,gene" << endl;
  for (vtkIdType row = 0; row < tab->GetNumberOfRows(); ++row)
    {
    vtkStdString cell = tab->GetValue(row, 0).ToString();
    for (vtkIdType col = 1; col < tab->GetNumberOfColumns(); ++col)
      {
      if (tab->GetValue(row, col).ToInt() == 1)
        {
        vtkStdString gene = tab->GetColumn(col)->GetName();
        cout << cell << "," << gene << endl;
        }
      }
    }
  return 0;
}
