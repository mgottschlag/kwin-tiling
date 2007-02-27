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

#include <kapplication.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include <qdesktopwidget.h>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QWidget>
//Added by qt3to4:
#include <QShowEvent>
#include <QFrame>

#include <objkstheme.h>
#include "themestandard.h"
#include "themestandard.moc"
#include "wndicon.h"
#include "wndstatus.h"

ThemeStandard::ThemeStandard( QWidget *parent, const QStringList &args )
  :ThemeEngine( parent, args ), mIcon(0L), mPrevIcon(0L), mIconCount(0), mStdIconWidth(-1),
  mIconPos(WndIcon::HBottomLeft), mSbAtTop(false), mSbVisible(true), mSbPbVisible(true), mSbFontName("helvetica"),
  mSbFontSz(16), mSbFontBold(true), mSbFontItalic(false), mSbFont(QFont()), mSbFg(QColor()), mSbBg(QColor()),
  mSbIcon("run"), mIconsVisible(true), mIconsJumping(true), mSplashScreen("(Default)")
{
  _readSettings();
  _initUi();
}

void ThemeStandard::_initUi()
{
  setFrameStyle( QFrame::NoFrame );

  QString pixName = mTheme->locateThemeData( mSplashScreen );

  if( mSplashScreen == "(Default)" || pixName.isEmpty() )
  {
    QString resource_prefix = "pics/";
    if ( mTheme->loColor() )
      resource_prefix += "locolor/";
    pixName = KStandardDirs::locate( "appdata", resource_prefix + "splash.png");
  }

  QPixmap px = QPixmap( pixName );

  if( !px.isNull() )
  {
    //kDebug() << "Loaded splash " << mSplashScreen << endl;
    int pw = px.width();
    int ph = px.height();

    QLabel *lbl = new QLabel( this );
    lbl->setAttribute(Qt::WA_NoSystemBackground, true);
    lbl->setFixedSize( pw, ph );
    lbl->setPixmap( px );

    resize( pw, ph );
  }
  else
  {
    //kDebug() << "Couldn't load splash " << mSplashScreen << endl;
    resize( 0, 0 );
  }

  const QRect rect = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );
  // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.

  move( rect.x() + (rect.width() - size().width())/2,
        rect.y() + (rect.height() - size().height())/2 );

  mStatus = new WndStatus( QPalette(), mTheme->xineramaScreen(), mSbAtTop, mSbPbVisible, mSbFont, mSbFg, mSbBg, mSbIcon );
}

void ThemeStandard::showEvent( QShowEvent * )
{

  ThemeEngine::show();

  if( mSbVisible )
  {
    mStatus->show();
    mStatusBarHeight = mStatus->height();
  }
  else
  {
    mStatusBarHeight = 0L;
  }
}

// Adjust the visible icon.
void ThemeStandard::slotSetPixmap( const QString& pxn )
{
  QPixmap px = DesktopIcon( pxn );

  if ( px.isNull() )
    px = DesktopIcon( "go" );

  if ( !mIconsVisible )
    return;

  /* (We only use prev_i if jumping is enabled...) */
  if ( mIconsJumping && mPrevIcon )
    emit mPrevIcon->slotStopJumping();

  if ( mStdIconWidth < 0 )
    mStdIconWidth = DesktopIcon( "go" ).width();

  mIcon = new WndIcon( ++mIconCount, mStdIconWidth, mStatusBarHeight, mTheme->xineramaScreen(),
                       px, QString(), mIconPos, mSbAtTop, mIconsJumping );
  mIcon->show();

  if( mIconsJumping )
  {
    emit mIcon->slotJump();
    mPrevIcon = mIcon;
  }
}

void ThemeStandard::_readSettings()
{

  if ( !mTheme )
    return;

  KConfig *cfg = mTheme->themeConfig();
  if ( !cfg )
    return;

  //if ( !cfg->hasGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) ) )
  //  return;
  KConfigGroup cg(cfg, QString("KSplash Theme: %1").arg(mTheme->theme()));

  QString sbpos = cg.readEntry( "Statusbar Position", "Bottom" ).toUpper();
  mSbAtTop = ( sbpos == "TOP" );
  mSbVisible = cg.readEntry( "Statusbar Visible", true);
  mSbPbVisible = cg.readEntry( "Progress Visible", true);

  mSbFontName = cg.readEntry( "Statusbar Font", "Sans Serif" );
  mSbFontSz = cg.readEntry( "Statusbar Font Size", 16 );
  mSbFontBold = cg.readEntry( "Statusbar Font Bold", true);
  mSbFontItalic = cg.readEntry( "Statusbar Font Italic", false);
  mSbFont = QFont( mSbFontName, mSbFontSz, ( mSbFontBold? QFont::Bold : QFont::Normal ) );
  if( mSbFontItalic )
    mSbFont.setItalic( true );

  QColor tmp = Qt::white;
  mSbFg = cg.readEntry( "Statusbar Foreground", tmp );
  tmp = Qt::black;
  mSbBg = cg.readEntry( "Statusbar Background", tmp );
  mSbIcon = cg.readEntry( "Statusbar Icon", "run" );
  mIconsVisible = cg.readEntry( "Icons Visible", true);
  mIconsJumping = cg.readEntry( "Icons Jumping", true);
  mIconPos = (WndIcon::Position)cg.readEntry( "Icon Position", 0 );
  mSplashScreen = cg.readEntry( "Splash Screen", "(Default)");
  // cg.readEntry( "Allow Configuration", QVariant(true )).toBool();
}

