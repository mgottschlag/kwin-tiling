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

#ifndef __THEMEREDMOND_H__
#define __THEMEREDMOND_H__

#include <kdebug.h>

#include <themeengine.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

class KFontCombo;
class QCheckBox;

class CfgRedmond: public ThemeEngineConfig
{
  Q_OBJECT
public:
  CfgRedmond( QWidget *, KConfig * );

protected:
  QCheckBox *mShowUsername;
  QCheckBox *mShowIcon;
  QCheckBox *mShowWelcome;
  KFontCombo *mWelcomeFont;
  KFontCombo *mUsernameFont;
  KFontCombo *mActionFont;
};

class ObjKsTheme;
class ThemeRedmond: public ThemeEngine
{
  Q_OBJECT
public:
  ThemeRedmond( QWidget *, const char *, const QStringList& );

  inline const QString name() { return( QString("Redmond") );  }
  static QStringList names()
  {
    QStringList Names;
    Names << "Redmond";
    return( Names );
  };

public Q_SLOTS:
  inline void slotSetText( const QString& s )
  {
    if( mText != s )
    {
      mText = s;
      repaint( false );
    }
  };

private:
  void paintEvent( QPaintEvent * );

  void _initUi();
  void _readSettings();

  QString mText;
  QPixmap mPixmap;
  bool mRedrawKonqi;
  QPoint mMsgPos;
  QPixmap mImage;

  // ThemeEngine configuration.
  bool mShowWelcomeText;
  bool mShowWelcomeTextShadow;
  bool mWelcomeFontItalic;
  bool mShowUsernameText;
  bool mShowActionText;
  bool mShowIcon;
  bool mUseKdmUserIcon;
  QString mBackgroundImage;
  QString mWelcomeText;
  QString mUsernameText; // Leave this undefined to autodetect the username.
  QString mIcon;
  QFont mWelcomeFont;
  QFont mUsernameFont;
  QFont mActionFont;
  QColor mWelcomeTextColor;
  QColor mWelcomeTextShadowColor;
  QColor mUsernameTextColor;
  QColor mActionTextColor;
  QPoint mWelcomeTextPosition; // Set this to (0,0) to autoposition the text.
  QPoint mUsernameTextPosition; // Likewise.
  QPoint mActionTextPosition; // Likewise likewise.
  QPoint mIconPosition; // ...

}
;

#endif
