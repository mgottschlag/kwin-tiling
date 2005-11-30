#ifndef _INFO_H_
#define _INFO_H_

#include <qwidget.h>
#include <QStackedWidget>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <q3listview.h>
#include <qfile.h>
#include <qevent.h>

#include <kcmodule.h>
#include <kaboutdata.h>

#include "config.h"

/* function call-back-prototypes... */

bool GetInfo_CPU( Q3ListView *lBox );
bool GetInfo_IRQ( Q3ListView *lBox );
bool GetInfo_DMA( Q3ListView *lBox );
bool GetInfo_PCI( Q3ListView *lBox );
bool GetInfo_IO_Ports( Q3ListView *lBox );
bool GetInfo_Sound( Q3ListView *lBox );
bool GetInfo_Devices( Q3ListView *lBox );
bool GetInfo_SCSI( Q3ListView *lBox );
bool GetInfo_Partitions( Q3ListView *lBox );
bool GetInfo_XServer_and_Video( Q3ListView *lBox );
extern bool GetInfo_OpenGL( Q3ListView *lBox );

class KInfoListWidget : public KCModule
{
public:
  KInfoListWidget(KInstance *inst,const QString &_title, QWidget *parent, bool _getlistbox (Q3ListView *)=0);

  virtual void load();
  virtual QString quickHelp() const;
  
private:
  Q3ListView 	*lBox;
  bool 		(*getlistbox) (Q3ListView *);
  QString title;
  
  QLabel	*NoInfoText;
  QString	ErrorString;
  QStackedWidget  *widgetStack;
};

#endif
