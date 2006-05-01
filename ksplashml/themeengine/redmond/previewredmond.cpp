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

#include <klocale.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QPaintEvent>

#include "previewredmond.h"
#include "previewredmond.moc"
/*
 * PreviewRedmond::PreviewRedmond(): Constructor. Set up some basic
 * things.
 */
PreviewRedmond::PreviewRedmond( QWidget* parent )
    :QWidget( parent ),

    /* Using direct constructors to prevent memory blit. */
    m_welcomeString (i18n("Welcome")),
    m_userString (i18n("(Your Name)")),

    m_welcomeFont (QFont( "Arial", 16, QFont::Bold )),
    m_userFont (QFont( "Arial", 16 )),
    m_statusFont (QFont( "Arial", 12, QFont::Bold )),

    m_welcomeColor (Qt::white),
    m_welcomeShadowColor (Qt::darkGray),
    m_userColor (Qt::darkGray),
    m_statusColor (Qt::white),

    m_icon (DesktopIcon("kmenu")),

    m_showWelcomeString (true),
    m_showUserString (true),
    m_showUserIcon (true),
    m_showStatusString (true)
{
  _updateCache();
}

void PreviewRedmond::paintEvent( QPaintEvent* pe )
{
  QPainter p;
  p.begin( this );
  p.drawPixmap( pe->rect(), m_cache );
  p.end();
}

void PreviewRedmond::resizeEvent( QResizeEvent *re )
{
  QWidget::resizeEvent( re );
  _updateCache();
}

// PreviewRedmond::_updateCache(): Based on our current settings, we need
// to adjust our cached image. We'll slick it and create a new QPixmap
// from size(), just to be sure we're not missing anything.
void PreviewRedmond::_updateCache()
{
  m_cache = QPixmap( size() );
  QPainter p;
  p.begin( &m_cache );

  p.fillRect( rect(), Qt::gray );

  m_welcomeFont.setItalic( true );
  p.setFont( m_welcomeFont );

  QPoint welcomeTextPos( rect().width()/2 - p.fontMetrics().width( m_welcomeString ),
    rect().height()/2 + p.fontMetrics().height()/2 );

  if( m_showWelcomeString )
  {
    p.setPen( m_welcomeShadowColor );
    p.drawText( welcomeTextPos.x()+2, welcomeTextPos.y()+2, m_welcomeString );
    p.setPen( m_welcomeColor );
    p.drawText( welcomeTextPos, m_welcomeString );
  }

  if( m_showUserString )
  {
    p.setPen( m_userColor );
    p.setFont( m_userFont );
    QPoint userTextPos( rect().width()/2 + m_icon.width() + 20,
      rect().height()/2 + p.fontMetrics().height()/2 );
    p.drawText( userTextPos, m_userString );
  }

  if( m_showUserIcon )
    p.drawPixmap( rect().width()/2 + 10, rect().height()/2, m_icon );

  if( m_showStatusString )
  {
    QPoint statusTextPos( rect().width()/2 + m_icon.width() + 20,
      rect().height()/2 + (int)(p.fontMetrics().height()*0.85) + 15 );
    p.setPen( m_statusColor );
    p.setFont( m_statusFont );
    p.drawText( statusTextPos, i18n("Starting KDE...") );
  }

  p.end();
  update( rect() );
}
