/* This is the default widget for kcc
   Author: Markus Wuebben
	   <markus.wuebben@kde.org>
   Date:   September '97         */


#include <unistd.h>
#include <stdio.h>

#include <kglobal.h>
#include <kcharsets.h>
#include <klocale.h>
#include <kiconloader.h>

#include "mainwidget.moc"
#include "mainwidget.h"

mainWidget::mainWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  KIconLoader iconLoader;

  QLabel *heading = new QLabel(i18n("KDE Control Center"),this);
  // FIXME: should use KDE fonts
  QFont font("times",18,QFont::Bold);
  KGlobal::charsets()->setQFont(font);
  pmap = iconLoader.loadIcon("kdekcc.xpm");
  heading->setFont(font);
  heading->adjustSize();
  heading->move(120,10);

  uname(&info);
}


void mainWidget::paintEvent(QPaintEvent *)
{
  QString str;
  char buf[512];
  QPainter p(this);
  
  // FIXME: should use KDE Fonts!!!
  QFont normalFont("times",12,QFont::Normal);
  KGlobal::charsets()->setQFont(normalFont);
  QFont boldFont("times",12,QFont::Bold);
  KGlobal::charsets()->setQFont(boldFont);

  // center the pixmap horizontally
  p.drawPixmap( (width() - pmap.width())/2, 250, pmap);
  p.setFont(boldFont);
  p.drawText(60,70, i18n("KDE Version: "));
  p.setFont(normalFont);
  p.drawText(180,70, KDE_VERSION_STRING);

  p.setFont(boldFont);
  p.drawText(60,90, i18n("User: "));
  p.setFont(normalFont);
  str = getlogin();
  p.drawText(180,90, str);

  p.setFont(boldFont);
  p.drawText(60,110, i18n("Hostname: "));
  gethostname(buf,511);
  p.setFont(normalFont);
  p.drawText(180,110, buf);

  p.setFont(boldFont);
  p.drawText(60,130, i18n("System: "));
  p.setFont(normalFont);
  p.drawText(180,130, info.sysname);
   
  p.setFont(boldFont);
  p.drawText(60,150, i18n("Release: "));
  p.setFont(normalFont);
  p.drawText(180,150, info.release);

  p.setFont(boldFont);
  p.drawText(60,170, i18n("Version: "));
  p.setFont(normalFont);
  p.drawText(180,170, info.version);

  p.setFont(boldFont);
  p.drawText(60,190, i18n("Machine: "));
  p.setFont(normalFont);
  p.drawText(180,190, info.machine);

  p.end();

}


void mainWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);

  emit resized();
}
