/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003  <ravi@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#ifndef THEMEENGINE_H
#define THEMEENGINE_H

#include <QStringList>
#include <kvbox.h>
#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>

#include <kdemacros.h>

class KConfig;
class ObjKsTheme;
class QMouseEvent;

/**
 * @short The base for the ThemeEngine's configuration widget.
 */
class KDE_EXPORT ThemeEngineConfig: public KVBox
{
  Q_OBJECT
public:

  ThemeEngineConfig( QWidget *p, KConfig *c )
      :KVBox( p ), mConfig( c )
  {}

  KConfig* config()const { return mConfig; }

public Q_SLOTS:
  virtual void load() {}
  virtual void save() {}

protected:
  KConfig *mConfig;
};

/**
 * @short Base class for all theme engines. Member functions need to be
 * overridden by derived classes in order to provide actual functionality.
 */
class KDE_EXPORT ThemeEngine: public QFrame
{
  Q_OBJECT
public:
  ThemeEngine( QWidget *parent, const QStringList &args );
  virtual ~ThemeEngine() = 0;
  virtual const ThemeEngineConfig *config( QWidget *, KConfig * ) { return 0L; }
  virtual ObjKsTheme *ksTheme() { return mTheme; }
  virtual bool eventFilter( QObject* o, QEvent* e );

public Q_SLOTS:
  virtual void slotUpdateProgress( int ) {}
  virtual void slotUpdateSteps( int ) {}
  virtual void slotSetText( const QString& ) {}
  virtual void slotSetPixmap( const QString& ) {} // use DesktopIcon() to load this.

protected:
  void addSplashWindow( QWidget* );

protected:
  ObjKsTheme *mTheme;
  virtual bool x11Event( XEvent* );

private Q_SLOTS:
  void splashWindowDestroyed( QObject* );

private:
  class ThemeEnginePrivate;
  ThemeEnginePrivate *d;
};

#endif
