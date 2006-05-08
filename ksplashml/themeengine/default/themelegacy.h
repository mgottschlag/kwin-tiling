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

#ifndef __THEMELEGACY_H__
#define __THEMELEGACY_H__

#include <qlabel.h>
#include <qwidget.h>
#include <QProgressBar>
//Added by qt3to4:
#include <QPixmap>

#include "themeengine.h"
class QPixmap;
class QTimer;

class QCheckBox;

class DefaultConfig: public ThemeEngineConfig
{
  Q_OBJECT
public:
  DefaultConfig( QWidget *, KConfig * );
  void save();
protected:
  QCheckBox *mFlash, *mAlwaysShow;
};

/**
 * @short Traditional KDE splash screen.
 */
class ObjKsTheme;
class KDE_EXPORT ThemeDefault : public ThemeEngine
{
  Q_OBJECT
public:
  ThemeDefault( QWidget *, const QStringList& );
   virtual ~ThemeDefault();

  inline const DefaultConfig *config( QWidget *p, KConfig *c )
  {
    return new DefaultConfig( p, c );
  };

  static QStringList names()
  {
    QStringList Names;
    Names << "Default";
    Names << "Classic";
    Names << "Klassic";
    return( Names );
  }

public Q_SLOTS:
  inline void slotSetText( const QString& s )
  {
    if( mLabel )
      mLabel->setText( s );
    slotUpdateState();
  };
  inline void slotUpdateSteps( int s )
  {
    mProgressBar->show();
    mProgressBar->setRange( 0, s );
  }
  inline void slotUpdateProgress( int i )
  {
    mProgressBar->setValue( i );
  }


private Q_SLOTS:
  void slotUpdateState();
  QPixmap updateBarPixmap( int );
  void flash();

private:
  void _initUi();
  void _readSettings();
  QString _findPicture( const QString &pic );

  // Configurable Options
  bool mIconsFlashing;
  QColor mLabelForeground;

  // Internals.
  QProgressBar *mProgressBar;
  QLabel *mLabel, *mBarLabel;
  QPixmap *mActivePixmap, *mInactivePixmap;
  int mState;
  QTimer *mFlashTimer;
  QPixmap *mFlashPixmap1, *mFlashPixmap2;
};

#endif
