#ifndef __QVCRWidget
#define __QVCRWidget

#include <QWidget>

class QVCRWidgetUi;

class QVCRWidget : public QWidget
{
  Q_OBJECT
public:
  explicit QVCRWidget( QWidget* parent = 0, Qt::WFlags = 0 );
  ~QVCRWidget();
signals:
  void first();
  void back();
  void play();
  void pause();
  void forward();
  void last();

private:
  QVCRWidgetUi* ui;
};
#endif

