#ifndef _MEMORY_H_KDEINFO_INCLUDED_
#define _MEMORY_H_KDEINFO_INCLUDED_


#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtabdialog.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kcmodule.h>

#ifdef __i386__
/* use long-long, because some 32bit-machines have more memory than 4GB (with Swap) */
typedef unsigned long long t_memsize;
#else
typedef unsigned long t_memsize;
#endif

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
  bool Display_Graph( int widgetindex, bool highlight,
		t_memsize total, t_memsize avail );
  
public slots:
  void update_Values();

};


#endif // _MEMORY_H_KDEINFO_INCLUDED_
