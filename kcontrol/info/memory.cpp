/* $Id$ */

#include <qtabbar.h>
#include <kglobal.h>
#include "memory.h"
#include <klocale.h>
#include "memory.moc"

#define STARTX  20
#define STARTX2 200
#define STARTY  20
#define DY      24

KMemoryWidget::KMemoryWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  totalMem = new QLabel(i18n("Total memory"), this);
  totalMem->move(STARTX,STARTY+0*DY);
  totalMem->setAutoResize(TRUE);
  totalMem = new QLabel("0k", this);
  totalMem->move(STARTX2,STARTY+0*DY);
  totalMem->setAutoResize(TRUE);
  QFont courierFont = KGlobal::fixedFont();
  totalMem->setFont(courierFont);

  freeMem = new QLabel(i18n("Free memory"), this);
  freeMem->move(STARTX,STARTY+1*DY);
  freeMem->setAutoResize(TRUE);
  freeMem = new QLabel("0k", this);
  freeMem->move(STARTX2,STARTY+1*DY);
  freeMem->setAutoResize(TRUE);
  freeMem->setFont(courierFont);

  sharedMem = new QLabel(i18n("Shared memory"), this);
  sharedMem->move(STARTX,STARTY+2*DY);
  sharedMem->setAutoResize(TRUE);
  sharedMem = new QLabel("0k", this);
  sharedMem->move(STARTX2,STARTY+2*DY);
  sharedMem->setAutoResize(TRUE);
  sharedMem->setFont(courierFont);

  bufferMem = new QLabel(i18n("Buffer memory"), this);
  bufferMem->move(STARTX,STARTY+3*DY);
  bufferMem->setAutoResize(TRUE);
  bufferMem = new QLabel("0k", this);
  bufferMem->move(STARTX2,STARTY+3*DY);
  bufferMem->setAutoResize(TRUE);
  bufferMem->setFont(courierFont);

  swapMem = new QLabel(i18n("Swap memory"), this);
  swapMem->move(STARTX,STARTY+5*DY);
  swapMem->setAutoResize(TRUE);
  swapMem = new QLabel("0k", this);
  swapMem->move(STARTX2,STARTY+5*DY);
  swapMem->setAutoResize(TRUE);
  swapMem->setFont(courierFont);

  freeSwapMem = new QLabel(i18n("Free swap memory"), this);
  freeSwapMem->move(STARTX,STARTY+6*DY);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem = new QLabel("0k", this);
  freeSwapMem->move(STARTX2,STARTY+6*DY);
  freeSwapMem->setAutoResize(TRUE);
  freeSwapMem->setFont(courierFont);

  timer = new QTimer(this);
  timer->start(100);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));

  update();
}


QString format(unsigned long value)
{
  double   mb = value / 1048576.0;
  return i18n("%1 bytes  = %2 MB").arg(value,10).arg(mb,10,'f',2);
}


// Include system-specific code

#ifdef linux
#include "memory_linux.cpp"
#elif sgi
#include "memory_sgi.cpp"
#elif __FreeBSD__
#include "memory_fbsd.cpp"
#elif hpux
#include "memory_hpux.cpp"
#else

// Default for unsupported systems

void KMemoryWidget::update()
{
  QString Not_Available_Text;

  Not_Available_Text = i18n("Not available");
  totalMem->setText(Not_Available_Text);
  freeMem->setText(Not_Available_Text);
  sharedMem->setText(Not_Available_Text);
  bufferMem->setText(Not_Available_Text);
  swapMem->setText(Not_Available_Text);
  freeSwapMem->setText(Not_Available_Text);
}

#endif

