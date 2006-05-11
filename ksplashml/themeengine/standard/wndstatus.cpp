/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kwin.h>

#include <qdesktopwidget.h>
#include <QLabel>
#include <QPoint>
#include <qrect.h>
#include <QProgressBar>
#include <QPixmap>

#include "wndstatus.h"
#include "wndstatus.moc"

// WndStatus::WndStatus(): Display a nifty status bar at
// the bottom of the screen, so the user always knows what's
// happening to his system.
WndStatus::WndStatus( QPalette /*pal*/,
                      int xineramaScreen,
                      bool atTop, bool pbVisible,
                      const QFont& font,
                      const QColor& fgc, const QColor & bgc,
                      const QString& icon
                    )
    :KHBox( 0)
{
  setWindowFlags(Qt::WStyle_Customize|Qt::WX11BypassWM);
  //setPalette( pal );
  setPaletteBackgroundColor( bgc );
  setPaletteForegroundColor( fgc );
  setCursor( KCursor::blankCursor() );
  setSpacing( 5 );

  const QRect rect = kapp->desktop()->screenGeometry( xineramaScreen );
  // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.

  QLabel *pix = new QLabel( this );
  QPixmap _icon( SmallIcon(icon.isNull()||icon.isEmpty()?QString("run"):icon) );
  pix->setPixmap( _icon );
  setStretchFactor(pix,0);
  pix->setFixedWidth(16);

  m_label = new QLabel( this );
  m_label->setFont( font );
  m_label->setPaletteBackgroundColor( bgc );
  m_label->setPaletteForegroundColor( fgc );
  //QFontMetrics metrics( font );
  //m_label->setFixedHeight( metrics.height() );
  m_label->setText(QString(""));
  m_label->setFixedWidth(rect.width()-105-16-10); // What's this magic number?
  m_label->show();

  m_progress = new QProgressBar( this );
  setStretchFactor(m_progress,0);
  m_progress->setFixedWidth(100);

  QWidget *widg = new QWidget( this );
  setStretchFactor(widg,50);

  setFixedSize( rect.width(), qMax(m_progress->height(),m_label->height()) );

  if ( atTop )
    move( rect.topLeft() );
  else
    move( rect.bottomLeft().x(), rect.bottomLeft().y()-height()+1 ); // The +1 is to work around a bug in screenGeometry().

  if (!pbVisible)
    m_progress->hide();
}

void WndStatus::slotSetMessage( const QString& msg )
{
  raise();
  m_label->setText( msg );
}

void WndStatus::slotUpdateProgress( int i )
{
  raise();
  m_progress->setValue( i );
}

void WndStatus::slotUpdateSteps( int i )
{
  m_progress->setRange( 0, i );
}
