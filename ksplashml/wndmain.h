/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003 <ravi@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#ifndef __WNDMAIN_H__
#define __WNDMAIN_H__

#include <kapplication.h>

#include <q3ptrlist.h>
#include <qstring.h>
#include <qobject.h>
//Added by qt3to4:
#include <QEvent>

#include "ksplashiface.h"

// MAKE SURE THAT THIS MATCHES WHAT'S IN ../kcmksplash/kcmksplash.h!!!
#define N_ACTIONITEMS 8

// Action: This represents an "action entry" to any object which is interested
// in knowing this.
typedef struct
{
  QString ItemPixmap;
  QString ItemText;
} Action;

class WndStatus;
class ObjKsTheme;
class ThemeEngine;
class KConfig;

class KSplash: public QWidget, virtual public KSplashIface
{
  Q_OBJECT

public:
  KSplash(const char *name = "ksplash");
  ~KSplash();

  Q3PtrList<Action> actionList();

  // DCOP interface
  ASYNC upAndRunning( QString );
  ASYNC setMaxProgress(int);
  ASYNC setProgress(int);
  ASYNC setStartupItemCount( int count );
  ASYNC programStarted( QString programIcon, QString programName, QString description );
  ASYNC startupComplete();
  ASYNC close();

Q_SIGNALS:
  void stepsChanged(int);
  void progressChanged(int);
  void actionListChanged();

protected:
  bool eventFilter( QObject *o, QEvent *e );

public Q_SLOTS:
  void slotUpdateSteps( int );
  void slotUpdateProgress( int );

private Q_SLOTS:
  void initDcop();
  void prepareIconList();
  void prepareSplashScreen();
  void slotExec();
  void nextIcon();
  void slotInsertAction( const QString&, const QString& );
  void slotReadProperties( KConfig * );

  void slotSetText( const QString& );
  void slotSetPixmap( const QString& );

  void loadTheme( const QString& );

private:
  ThemeEngine *_loadThemeEngine( const QString& pluginName, const QString& theme );
  void updateState( unsigned int state );

protected:
  unsigned int mState;
  unsigned int mMaxProgress;
  unsigned int mStep; // ??
  QTimer* close_timer;

  bool mSessMgrCalled;
  bool mTimeToGo;

  QString mTheme;
  ObjKsTheme *mKsTheme;

  ThemeEngine *mThemeEngine;
  Q3PtrList<Action> mActionList;
  Action *mCurrentAction, *mPreviousAction;

  QString mThemeLibName;
};

#endif
