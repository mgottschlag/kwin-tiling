#ifndef _MEMORY_H_KDEINFO_INCLUDED_
#define _MEMORY_H_KDEINFO_INCLUDED_


#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtabdialog.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kcmodule.h>


class KMemoryWidget : public KCModule
{
  Q_OBJECT

public:
  KMemoryWidget(QWidget *parent, const char *name=0);
  ~KMemoryWidget();

private:
  QString Not_Available_Text;
  QTimer *timer;
  
  void update();
  int  Width_Info,
       Width_Value,
       Width_NoInfo;
  
public slots:
  void update_Values();

protected:
  virtual void resizeEvent( QResizeEvent * );
};


#endif // _MEMORY_H_KDEINFO_INCLUDED_
