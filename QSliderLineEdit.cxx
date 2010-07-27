// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "QSliderLineEdit"

#include "QLayout"
#include "QLineEdit"
#include "QSizePolicy"
#include "QSlider"

#include <math.h>

//-----------------------------------------------------------------------------

QSliderLineEdit::QSliderLineEdit(QWidget *p, Qt::WFlags f)
  : QWidget(p, f)
{
  QHBoxLayout *hLayout = new QHBoxLayout(this);
  hLayout->setMargin(0);
  hLayout->setSpacing(0);

  this->slider = new QSlider(this);
  this->slider->setOrientation(Qt::Horizontal);
  this->slider->setTracking(false);
  connect(this->slider, SIGNAL(sliderMoved(int)), 
          this, SLOT(slotSliderMoved(int)));
  connect(this->slider, SIGNAL(valueChanged(int)),
          this, SLOT(interactionDone()));
  hLayout->addWidget(this->slider);

  this->lineEdit = new QLineEdit(this);
  this->lineEdit->setAlignment(Qt::AlignRight);
  connect(this->lineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(textChanged(const QString &)));
  connect(this->lineEdit, SIGNAL(returnPressed()),
	  this, SLOT(interactionDone()));
  hLayout->addWidget(this->lineEdit);

  QSizePolicy sizepolicy;
  sizepolicy.setHorizontalPolicy(QSizePolicy::Preferred);
  sizepolicy.setVerticalPolicy(QSizePolicy::Fixed);
  sizepolicy.setHorizontalStretch(3);
  sizepolicy.setVerticalStretch(0);
  this->slider->setSizePolicy(sizepolicy);

  sizepolicy.setHorizontalPolicy(QSizePolicy::Expanding);
  sizepolicy.setVerticalPolicy(QSizePolicy::Fixed);
  sizepolicy.setHorizontalStretch(1);
  sizepolicy.setVerticalStretch(0);
  this->lineEdit->setSizePolicy(sizepolicy);

  this->_value = 0;
  this->_scalingType = QtSNLCommon::IntegerScale;
  this->_minimum = 0;
  this->_maximum = 1;
  this->_clampValue = true;
  this->_tracking = false;

  this->inSetValue = false;
  this->inSetValueInteractive = false;
  this->inSliderMoved = false;
  this->inTextChanged = false;

  this->setupSlider();
  this->lineEdit->setText("0");
}

QSliderLineEdit::~QSliderLineEdit()
{
  // No need to delete child widgets.  Qt does that automatically.
}

//-----------------------------------------------------------------------------

int QSliderLineEdit::valueToSliderPos(double v)
{
  // Integers are an easy special case.
  if (this->scalingType() == QtSNLCommon::IntegerScale)
    {
    return (int)v;
    }

  double min = this->minimum();
  double max = this->maximum();
  switch (this->scalingType())
    {
    case QtSNLCommon::LogarithmicScale:
      min = ((min < 1.0e-300) ? log(1.0e-300) : log(min));
      max = ((max < 1.0e-300) ? log(1.0e-300) : log(max));
      v = ((v < 1.0e-300) ? log(1.0e-300) : log(v));
      break;
    case QtSNLCommon::ExponentialScale:
      min = ((min > 690.0) ? exp(690.0) : exp(min));
      max = ((max > 690.0) ? exp(690.0) : exp(max));
      v = ((v > 690.0) ? exp(690.0) : exp(v));
      break;
    default:
      // Do nothing.
      break;
    }

  return (int)(1000*((v-min)/(max-min)));
}

double QSliderLineEdit::sliderPosToValue(int position)
{
  // Integers are an easy special case.
  if (this->scalingType() == QtSNLCommon::IntegerScale)
    {
    return (int)position;
    }

  double min = this->minimum();
  double max = this->maximum();
  switch (this->scalingType())
    {
    case QtSNLCommon::LogarithmicScale:
      min = ((min < 1.0e-300) ? log(1.0e-300) : log(min));
      max = ((max < 1.0e-300) ? log(1.0e-300) : log(max));
      break;
    case QtSNLCommon::ExponentialScale:
      min = ((min > 690.0) ? exp(690.0) : exp(min));
      max = ((max > 690.0) ? exp(690.0) : exp(max));
      break;
    default:
      // Do nothing.
      break;
    }

  // Scaled value.
  double sv = ((double)position/1000.0f)*(max-min) + min;

  switch (this->scalingType())
    {
    case QtSNLCommon::IntegerScale: return 0.0f;      // Cannot reach this line.
    case QtSNLCommon::LinearScale: return sv;
    case QtSNLCommon::LogarithmicScale: return (double)exp(sv);
    case QtSNLCommon::ExponentialScale: return (double)log(sv);
    }

  // Shut up, compiler.
  return 0.0;
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::setupSlider()
{
  this->slider->setMinimum(this->valueToSliderPos(this->minimum()));
  this->slider->setMaximum(this->valueToSliderPos(this->maximum()));
  this->slider->setValue(this->valueToSliderPos(this->value()));
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::setScalingType(QtSNLCommon::ScalingType type)
{
  if (type == this->_scalingType) return;

  this->_scalingType = type;
  this->setupSlider();
}

//-----------------------------------------------------------------------------

double QSliderLineEdit::clamp(double v)
{
  if (!this->clampValue()) return v;

  if (this->_minimum <= this->_maximum)
    {
    if (v < this->_minimum)
      {
      return this->_minimum;
      }
    else if (v > this->_maximum)
      {
      return this->_maximum;
      }
    return v;
    }
  else
    {
    if (v < this->_maximum)
      {
      return this->_maximum;
      }
    else if (v > this->_minimum)
      {
      return this->_minimum;
      }
    return v;
    }
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::setMinimum(double v)
{
  if (v == this->_minimum) return;

  this->_minimum = v;
  this->setupSlider();
}

void QSliderLineEdit::setMaximum(double v)
{
  if (v == this->_maximum) return;

  this->_maximum = v;
  this->setupSlider();
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::setValue(double v)
{
  v = this->clamp(v);
  if (v == this->_value) return;
  if (this->inSetValue) return;
  this->inSetValue = true;

  this->setValueInteractive(v);
  if (!this->isTracking())
    {
    emit valueChanged(v);
    emit valueChanged((int)v);
    }

  this->inSetValue = false;
}

void QSliderLineEdit::setValue(int v)
{
  this->setValue((double)v);
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::setValueInteractive(double v)
{
  v = this->clamp(v);
  if (v == this->_value) return;
  if (this->inSetValueInteractive) return;
  this->inSetValueInteractive = true;

  this->_value = v;
  if (!this->inSliderMoved) this->slider->setValue(this->valueToSliderPos(v));
  if (!this->inTextChanged) this->lineEdit->setText(QString::number(v));

  if (this->isTracking())
    {
    emit valueChanged(v);
    emit valueChanged((int)v);
    }

  this->inSetValueInteractive = false;
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::slotSliderMoved(int position)
{
  if (this->inSliderMoved) return;
  this->inSliderMoved = true;
  this->setValueInteractive(this->sliderPosToValue(position));
  emit sliderMoved(this->value());
  emit sliderMoved((int)this->value());
  this->inSliderMoved = false;
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::textChanged(const QString &text)
{
  if (this->inTextChanged) return;
  this->inTextChanged = true;
  this->setValueInteractive(text.toDouble());
  this->inTextChanged = false;
}

//-----------------------------------------------------------------------------

void QSliderLineEdit::interactionDone()
{
  if (!this->inSetValueInteractive && !this->isTracking())
    {
    emit valueChanged(this->value());
    emit valueChanged((int)this->value());
    }
}
