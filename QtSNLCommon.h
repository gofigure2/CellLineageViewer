// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

/*
** This class contains enumerations for objects in the QtSNLCommon library.
** Putting them here allows objects to share enumerations.
*/

#ifndef _QtSNLCommon_h
#define _QtSNLCommon_h

#include <QObject>

class QtSNLCommon : public QObject
{
  Q_OBJECT;
public:

  // Description:
  // Ways to scale a variable.
  Q_ENUMS(ScalingType);
  enum ScalingType {
    IntegerScale,
    LinearScale,
    LogarithmicScale,
    ExponentialScale
  };

  // Description:
  // Possible variable types.
  Q_ENUMS(VariableType);
  enum VariableType {
    PointVariable,
    CellVariable,
    InvalidVariable
  };
  
};

#endif //_QtSNLCommon_h
