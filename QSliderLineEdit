// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#ifndef _QSliderLineEdit_h
#define _QSliderLineEdit_h

#include "qwidget.h"

#include "QtSNLCommon.h"

class QLineEdit;
class QSlider;

// .SECTION Name QSliderLineEdit
//
// .SECTION Description
// This class is a slider coupled with a line edit.  Either widget can be
// used to set the value.  The value itself can be floating point and will
// automatically be scaled.  Currently, the slider is horizontal.

class QSliderLineEdit : public QWidget
{
  Q_OBJECT;

  /// The current value of the slider.
  Q_PROPERTY(double value READ value WRITE setValue);
  /// Minimum value of the slider.
  Q_PROPERTY(double minimum READ minimum WRITE setMinimum);
  /// Maximum value of the slider.
  Q_PROPERTY(double maximum READ maximum WRITE setMaximum);

  /// If true (the default), values will be clamped between the left and
  /// right values of the slider when the user enters a value in the line
  /// edit outside of that range.
  Q_PROPERTY(bool clampValue READ clampValue WRITE setClampValue);

  /// If true, the valueChanged signal is called while the slider is dragged
  /// or the textbox is edited.  By default, turned off.
  Q_PROPERTY(bool tracking READ isTracking WRITE setTracking);

  /// Determines the type of scaling performed on the scalar bar.  The types
  /// have the following meanings:
  ///   LinearScale (default): A linear range from left value to right value.
  ///   IntegerScale: The integers between left value and right value
  //     (each rounded down to the nearest integer).
  ///   LogarithmicScale: The slider will have a logarithmic scale.  Values
  ///     less than 1e-38 will not be represented by the slider.
  ///   ExponentialScale: The slider will have an exponential scale.  Values
  ///     greater than 85 will not be represented by the slider.
  Q_PROPERTY(QtSNLCommon::ScalingType scalingType
             READ scalingType WRITE setScalingType);

  /// Determines the orientation of the scalar bar.  Currently only
  /// Qt::Horizontal is supported.
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation);

  Q_ENUMS(QtSNLCommon::ScalingType);
  Q_ENUMS(Qt::Orientation);

public:
  QSliderLineEdit(QWidget *parent = NULL, Qt::WFlags f = 0);
  virtual ~QSliderLineEdit();

  /// The value.
  inline double value() const { return this->_value; }

  /// See the scalingType property.
  inline QtSNLCommon::ScalingType scalingType() const
    { return this->_scalingType; }
  void setScalingType(QtSNLCommon::ScalingType type);

  /// See minimum and maximum properties.
  inline double minimum() const { return this->_minimum; }
  inline double maximum() const { return this->_maximum; }

  /// Set the minimum and maximum properties in one method.
  void setRange(double min, double max) {
    this->setMinimum(min);  this->setMaximum(max);
  }
  void setRange(int min, int max) { this->setRange((double)min, (double)max); }

  /// See clampValue property.
  inline bool clampValue() const { return this->_clampValue; }
  void setClampValue(bool flag) { this->_clampValue = flag; }

  /// See tracking property.
  inline bool isTracking() const { return this->_tracking; }
  void setTracking(bool flag) { this->_tracking = flag; }

  /// See orientation property.
  inline Qt::Orientation orientation() const { return Qt::Horizontal; }
  void setOrientation(Qt::Orientation o) {
    if (o != Qt::Horizontal)
      {
      qCritical("Only Horizontal QScalarLineEdits are currently implemented.");
      }
  }

  /// Same as setValue.  This is a convenience method to mimic the interface
  /// to QSlider.
  void setSliderPosition(double v) { this->setValue(v); }

signals:
  /// Emitted whenever the user selects a new value by releasing the slider
  /// or hitting enter in the line edit.
  void valueChanged(double value);
  /// Same as above, but cast to an int.
  void valueChanged(int value);

  /// Emitted whenever the user is dragging the slider
  /// or hitting enter in the line edit.
  void sliderMoved(double pos);
  /// Same as above, but cast to an int.
  void sliderMoved(int pos);

public slots:
  /// Set the value of the slider and line edit.
  virtual void setValue(double value);
  /// Same as above, but cast to an int.
  virtual void setValue(int value);
  /// See minimum and maximum properties.
  void setMinimum(double value);
  void setMaximum(double value);

protected:
  QSlider *slider;
  QLineEdit *lineEdit;

  int valueToSliderPos(double v);
  double sliderPosToValue(int p);

  void setupSlider();

  double clamp(double v);

protected slots:
  virtual void setValueInteractive(double v);
  virtual void slotSliderMoved(int pos);
  virtual void textChanged(const QString &text);
  virtual void interactionDone();

private:
  QSliderLineEdit(const QSliderLineEdit &);   // Not implemented
  void operator=(const QSliderLineEdit &);     // Not implemented

  double _value;
  QtSNLCommon::ScalingType _scalingType;
  double _minimum;
  double _maximum;
  bool _clampValue;
  bool _tracking;

  bool inSetValue;
  bool inSetValueInteractive;
  bool inSliderMoved;
  bool inTextChanged;
};

#endif //_QSliderLineEdit_h
