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

#ifndef __PREVIEWREDMOND_H__
#define __PREVIEWREDMOND_H__

#include <kiconloader.h>

#include <QColor>
#include <QFont>
#include <QWidget>
//Added by qt3to4:
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>

/*
 * class PreviewRedmond: Provides a sneak peek at how certain Redmond
 * settings will look. This will not be able to render any background
 * images, so we'll just use a nice shade of gray or black as the
 * background...
 */
class PreviewRedmond: public QWidget
{
  Q_OBJECT
public:

  PreviewRedmond( QWidget* );

  inline void setWelcomeString( const QString& s )
  {
    m_welcomeString = s;
    _updateCache();
  }
  inline void setUserString( const QString& s )
  {
    m_userString = s;
    _updateCache();
  }

  inline void setWelcomeFont( const QFont& f )
  {
    m_welcomeFont = f;
    _updateCache();
  }
  inline void setUserFont( const QFont& f )
  {
    m_userFont = f;
    _updateCache();
  }
  inline void setStatusFont( const QFont& f )
  {
    m_statusFont = f;
    _updateCache();
  }

  inline void setWelcomeColor( const QColor& c )
  {
    m_welcomeColor = c;
    _updateCache();
  }
  inline void setWelcomeShadowColor( const QColor& c )
  {
    m_welcomeShadowColor = c;
    _updateCache();
  }
  inline void setUserColor( const QColor& c )
  {
    m_userColor = c;
    _updateCache();
  }
  inline void setStatusColor( const QColor& c )
  {
    m_statusColor = c;
    _updateCache();
  }

  inline void setIcon( const QString& s )
  {
    m_icon = DesktopIcon( s );
    _updateCache();
  }

protected:
  void _updateCache();
  void paintEvent( QPaintEvent* );
  void resizeEvent( QResizeEvent* );

  QPixmap m_cache;

  QString m_welcomeString, m_userString;
  QFont m_welcomeFont, m_userFont, m_statusFont;
  QColor m_welcomeColor, m_welcomeShadowColor, m_userColor, m_statusColor;
  QPixmap m_icon;

  bool m_showWelcomeString, m_showUserString, m_showUserIcon, m_showStatusString;
};

#endif
