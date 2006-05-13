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

#ifndef __OBJKSTHEME_H__
#define __OBJKSTHEME_H__

#include <kcmdlineargs.h>

#include <QString>
#include <QStringList>

class KConfig;
/**
 * @short Theme reader.
 * A Theme is read in from the file
 * <KDEDIR>/share/apps/ksplash/Themes/<theme>/Theme.rc
 * This controls the behavior, graphics, and appearance
 * of KSplash completely, and offers a friendlier way
 * of installing custom splash screens.
 */
class KDE_EXPORT ObjKsTheme : public QObject
{
  Q_OBJECT
public:
  explicit ObjKsTheme( const QString& );
  virtual ~ObjKsTheme();

  void loadCmdLineArgs( KCmdLineArgs * );

  QString theme() const      { return( mActiveTheme ); }
  QString themeEngine() const { return( mThemeEngine ); }
  KConfig *themeConfig() const { return( mThemeConfig ); }
  QString themeDir() const   { return( mThemeDir );    }
  bool loColor() const       { return( mLoColor );     }
  bool testing() const       { return( mTesting );     }
  bool managedMode() const   { return( mManagedMode ); }
  QString icon( int i ) { return (m_icons[i-1].isNull()?(QString::null):m_icons[i-1]); }
  QString text( int i ) { return (m_text[i-1].isNull()?(QString::null):m_text[i-1]);   }
  QString locateThemeData( const QString &resource );
  int xineramaScreen() const { return mXineramaScreen; }

protected:
  bool loadThemeRc( const QString&, bool );
  bool loadLocalConfig( const QString&, bool );
  bool loadKConfig( KConfig *, const QString&, bool );

private:
  QString mActiveTheme, mThemeDir;
  KConfig *mThemeConfig;

  int mXineramaScreen;
  bool mLoColor, mTesting, mManagedMode;
  QString mThemeEngine;
  QString mThemePrefix;

  QStringList m_icons, m_text;

  class ObjKsThemePrivate;
  ObjKsThemePrivate *d;
};

#endif
