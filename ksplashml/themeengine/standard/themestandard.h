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

#ifndef __THEMESTANDARD_H__
#define __THEMESTANDARD_H__

#include <themeengine.h>

#include "wndicon.h"
#include "wndstatus.h"
//Added by qt3to4:
#include <QShowEvent>

/*
 * Special Note: The "Standard" engine is treated a little bit differently than
 * the other ThemeEngines in this program. Because we don't ever want to end up
 * in a situation in which there is _no_ user interface, this ThemeEngine will
 * be statically linked in with the ksplash binary itself, so that no dynamic
 * loading is necessary to access it. The disadvantage that this presents is
 * immediately obvious: The control center module is no longer able to dynamically
 * load a configuration object for this theme, since this engine doesn't reside in
 * a shared-object library like the other ones do. Therefore, we will have to
 * implement the ThemeEngineConfig object for this engine in the kcmksplash
 * subsystem, in such a manner that it is statically linked in with the control
 * panel. I know, this is really nasty, but I would rather have slightly uglier
 * code, so long as it makes the End User Experience of this program somewhat more
 * predictable (i.e., even if we can't do exactly what the user wants, we can still
 * do _something_...)
 *
 * Therefore, you will find the CfgStandard class in the ../kcmksplash/ directory.
 */

/**
 * @short The default KSplash splash screen.
 */
class ThemeStandard: public ThemeEngine
{
  Q_OBJECT
public:
  ThemeStandard( QWidget *, const char*, const QStringList& );
  //inline const ThemeEngineConfig *config( QWidget *p, KConfig *kc ) { return 0L; }

public Q_SLOTS:
  inline void slotUpdateProgress( int i )
  {
    if( mStatus ) mStatus->slotUpdateProgress( i );
  }
  inline void slotUpdateSteps( int i )
  {
    if( mStatus ) mStatus->slotUpdateSteps( i );
  }
  inline void slotSetText( const QString& s )
  {
    if( mStatus ) mStatus->slotSetMessage( s );
  }
  void slotSetPixmap( const QString& );

private:
  void showEvent( QShowEvent * );
  void _readSettings();
  void _initUi();

  WndStatus *mStatus;
  WndIcon *mIcon, *mPrevIcon;
  int mIconCount;
  int mStatusBarHeight;
  int mStdIconWidth;

  WndIcon::Position mIconPos;
  bool mSbAtTop;
  bool mSbVisible;
  bool mSbPbVisible;
  QString mSbFontName;
  int mSbFontSz;
  bool mSbFontBold;
  bool mSbFontItalic;
  QFont mSbFont;
  QColor mSbFg;
  QColor mSbBg;
  QString mSbIcon;
  bool mIconsVisible;
  bool mIconsJumping;
  QString mSplashScreen;
};

#endif
