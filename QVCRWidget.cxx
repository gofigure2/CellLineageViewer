// Copyright 2009 Sandia Corporation, Kitware Inc.
// See LICENSE.txt for details.

#include "QVCRWidget.h"

#include "ui_QVCRWidget.h"

#include <QPushButton>

class QVCRWidgetUi : public Ui::QVCRWidget {};

QVCRWidget::QVCRWidget( QWidget *p, Qt::WFlags f ) : QWidget( p, f )
{
  this->ui = new QVCRWidgetUi;
  this->ui->setupUi(this);

  connect( this->ui->firstButton, SIGNAL( released() ), SIGNAL( first() ) );
  connect( this->ui->backButton, SIGNAL( released() ), SIGNAL( back() ) );
  connect( this->ui->playButton, SIGNAL( released() ), SIGNAL( play() ) );
  connect( this->ui->pauseButton, SIGNAL( released() ), SIGNAL( pause() ) );
  connect( this->ui->forwardButton, SIGNAL( released() ), SIGNAL( forward() ) );
  connect( this->ui->lastButton, SIGNAL( released() ), SIGNAL( last() ) );
}

QVCRWidget::~QVCRWidget()
{
  delete this->ui;
}
