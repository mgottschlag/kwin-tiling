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

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kfontcombo.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuser.h>
#include <kemailsettings.h>
#include <kvbox.h>
#include <khbox.h>

#include <qcheckbox.h>
#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <QFrame>

#include <objkstheme.h>
#include "themeredmond.h"
#include "previewredmond.h"
#include "themeredmond.moc"

K_EXPORT_COMPONENT_FACTORY( ksplashredmond, KGenericFactory<ThemeRedmond>( "ksplash" ) )

CfgRedmond::CfgRedmond( QWidget *p, KConfig *c )
  :ThemeEngineConfig( p, c )
{
  KVBox *vbox = new KVBox( this );
  vbox->setSpacing( KDialog::spacingHint() );

  QFont defaultFont( "Arial", 48, QFont::Bold );
  defaultFont.setItalic( true );
  QFont defaultUsernameFont( "Arial", 16, QFont::Bold );
  QFont defaultActionFont( "Arial", 12, QFont::Bold );
  QColor defaultDarkColor( 3, 47, 156 );
  QColor defaultWhiteColor( Qt::white );

  KHBox *hbox = new KHBox( vbox );
  hbox->setFrameStyle( QFrame::WinPanel );
  hbox->setFrameShadow( QFrame::Sunken );
  PreviewRedmond* _preview = new PreviewRedmond( hbox );
  _preview->setFixedSize( 320, 200 );

  _preview->setWelcomeString( c->readEntry( "Welcome Text", i18n("Welcome") ) );

  _preview->setWelcomeFont( c->readEntry( "Welcome Font", defaultFont ) );
  _preview->setUserFont( c->readEntry( "Username Font", defaultUsernameFont ) );
  _preview->setStatusFont( c->readEntry( "Action Font", defaultActionFont ) );

  _preview->setWelcomeColor( c->readEntry( "Welcome Text Color", defaultWhiteColor ) );
  _preview->setWelcomeShadowColor( c->readEntry( "Welcome Shadow Color", defaultDarkColor ) );
  _preview->setUserColor( c->readEntry( "Username Text Color", defaultWhiteColor ) );
  _preview->setStatusColor( c->readEntry( "Action Text Color", defaultDarkColor ) );

  _preview->setIcon( c->readEntry( "User Icon", "kmenu" ) );

  QLabel *lbl = new QLabel( vbox );
  lbl->setText( i18n("(Sorry, but I haven't finished writing this one yet...)") );
}

ThemeRedmond::ThemeRedmond( QWidget *parent, const QStringList &args )
    :ThemeEngine( parent, args )
{
  _readSettings();
  _initUi();
}

void ThemeRedmond::_initUi()
{
  const QRect screen = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );
  //QRect fullScreen = KGlobalSettings::desktopGeometry(0L);

  mImage = QPixmap( screen.width(), screen.height() );

  QPainter p;
  p.begin( &mImage );
  p.fillRect( screen, QColor(3,47,156) );
  p.setPen( mActionTextColor );

  QString bgimg;

  // Start by seeing if the theme prefers a particular image.
  if( !mBackgroundImage.isEmpty() )
    bgimg = mTheme->locateThemeData( mBackgroundImage );

  /*
  * See if there is a resolution-specific background in THEMEDIR
  * before looking for the "generic" one. Having a Background.png
  * file for each resolution will greatly reduce the amount of time
  * it takes to initialize this ThemeEngine when running, due to
  * the fact that no scaling will be necessary to display the image.
  *
  * File must be named THEMEDIR/Background-WWWxHHH.png -- for example,
  * Mytheme/Background-1024x768.png
  *
  * ADDITIONAL NOTE: The resolution you specify will be obtained from
  * the PRIMARY SCREEN ONLY when running in XINERAMA mode. Be sure to
  * provide backgrounds using common resolutions (I recommend at least
  * providing 640x480 [unofficially unsupported by KDE], 800x600, and
  * 1024x768 images.)
  */
  if( bgimg.isEmpty() )
    bgimg = mTheme->locateThemeData( QString( "Background-%2x%3.png" ).arg( screen.width() ).arg( screen.height() ) );

  // If that can't be found, look for THEMEDIR/Background.png
  if( bgimg.isNull() && !mTheme->themeDir().isNull() )
    bgimg = mTheme->locateThemeData( "Background.png" );

  if( mPixmap.isNull() )
    mPixmap = DesktopIcon( "kmenu", 48 );

  QPixmap pix( bgimg );

  if( !pix.isNull() )
  {

    QPixmap tmp( QSize(screen.width(), screen.height() ) );
    float sw = (float)screen.width() / pix.width();
    float sh = (float)(screen.height()) / pix.height();

    QMatrix matrix;
    matrix.scale( sw, sh );
    tmp = pix.transformed( matrix );

    p.drawPixmap( 0, 0, tmp );
  }

  QFont f = mWelcomeFont;
  if( mWelcomeFontItalic )
    f.setItalic( true ); // this SHOULD BE stored in the QFont entry, dang it.
  p.setFont( f );
  QFontMetrics met( f );
  QSize fmet = met.size( 0L, mWelcomeText );

  // Paint the "Welcome" message, if we are instructed to. Optionally dispense with the
  // shadow.
  if ( mShowWelcomeText )
  {
    if( mWelcomeTextPosition == QPoint( 0, 0 ) )
    {
      mWelcomeTextPosition = QPoint( (screen.width()/2) - fmet.width() - 25,
              (screen.height()/2) - (fmet.height()/2) + fmet.height() );
    }
  }

  if( mShowWelcomeText )
  {
    if( mShowWelcomeTextShadow )
    {
      p.setPen( mWelcomeTextShadowColor );
      p.drawText( mWelcomeTextPosition+QPoint(2,2), mWelcomeText );
    }
    p.setPen( mWelcomeTextColor );
    p.drawText( mWelcomeTextPosition, mWelcomeText );
  }

  // The current theme wants to say something in particular, rather than display the
  // account's fullname.
  KUser user;
  QString greetingString = ( !mUsernameText.isNull() ) ? mUsernameText : user.fullName();
  // when we use KUser (system account data) we should also check KEMailSettings (e-mail settings and kcm_useraccount)
  // people often write real names only in e-mail settings
  if ( greetingString.isEmpty() )
  {
    KEMailSettings kes;
    greetingString = kes.getSetting( KEMailSettings::RealName );
  }

  // Try to load the user's KDM icon... TODO: Make this overridable by the Theme.
  if( mUseKdmUserIcon )
  {
    const QString defSys( ".default.face.icon" );  // The system-wide default image
    const int fAdminOnly  = 1;
    const int fAdminFirst = fAdminOnly+1;
    const int fUserFirst  = fAdminFirst+1;
    const int fUserOnly   = fUserFirst+1;

    int faceSource = fAdminOnly;
    KConfig *kdmconfig = new KConfig("kdm/kdmrc", true);
    kdmconfig->setGroup("X-*-Greeter");
    QString userPicsDir = kdmconfig->readEntry( "FaceDir", KGlobal::dirs()->resourceDirs("data").last() + "kdm/faces" ) + '/';
    QString fs = kdmconfig->readEntry( "FaceSource" );
    if (fs == QString::fromLatin1("UserOnly"))
      faceSource = fUserOnly;
    else if (fs == QString::fromLatin1("PreferUser"))
      faceSource = fUserFirst;
    else if (fs == QString::fromLatin1("PreferAdmin"))
      faceSource = fAdminFirst;
    else
      faceSource = fAdminOnly; // Admin Only
    delete kdmconfig;

    QPixmap userp;
    if ( faceSource == fAdminFirst )
    {
      // If the administrator's choice takes preference
      userp = QPixmap( userPicsDir + user.loginName() + ".face.icon" );
      if ( userp.isNull() )
        faceSource = fUserOnly;
    }
    if ( faceSource >= fUserFirst)
    {
      // If the user's choice takes preference
      userp = QPixmap( user.homeDir() + "/.face.icon" );
      if ( userp.isNull() && faceSource == fUserFirst ) // The user has no face, should we check for the admin's setting?
        userp = QPixmap( userPicsDir + user.loginName() + ".face.icon" );
      if ( userp.isNull() )
        userp = QPixmap( userPicsDir + defSys );
    }
    else if ( faceSource <= fAdminOnly )
    {
      // Admin only
      userp = QPixmap( userPicsDir + user.loginName() + ".face.icon" );
      if ( userp.isNull() )
        userp = QPixmap( userPicsDir + defSys );
    }
    if( !userp.isNull() )
      mPixmap = userp;
  }

  if( mShowIcon )
  {
    QPoint pos = mIconPosition;
    if( pos == QPoint( 0, 0 ) )
    {
      pos = QPoint( (screen.width()/2) + 10, (screen.height()/2) );
    }
    p.drawPixmap( pos, mPixmap );
  }

  // User name font. Leave this nailed-up for now.
  f = mUsernameFont;
  p.setFont( f );
  met = QFontMetrics( f );
  fmet = met.size( 0L, greetingString );

  if( mShowUsernameText )
  {
    QPoint pos = mUsernameTextPosition;
    if( pos == QPoint( 0, 0 ) )
    {
      pos = QPoint(
              (screen.width()/2) + mPixmap.width() + 20,
              (screen.height()/2) - (fmet.height()/2) + fmet.height()
            );
    }
    p.setPen( mUsernameTextColor );
    p.drawText( pos, greetingString );
  }

  p.end();

  setFixedSize( screen.width(), screen.height() );
  move( screen.topLeft() );
}

void ThemeRedmond::paintEvent( QPaintEvent *pe )
{
  const QRect screen = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );

  QPainter p;
  p.begin( this );

  QRect r = pe->rect();

  bitBlt( this, r.x(), r.y(),
          &mImage, r.x(), r.y(), r.width(), r.height() );

  if (mShowActionText)
  {
    p.setPen( mActionTextColor );
    QFont f = mActionFont;
    p.setFont( f );
    QFontMetrics met( f );
    QSize fmet = met.size( 0L, mText );

    mMsgPos = mActionTextPosition;
    if( mMsgPos == QPoint( 0, 0 ) )
    {
      mMsgPos = QPoint(
        (screen.width()/2) + mPixmap.width() + 20,
        (screen.height()/2) + (int)(fmet.height()*0.85) + 15
        );
    }
    p.drawText( mMsgPos, mText );
  }
  p.end();
}

void ThemeRedmond::_readSettings()
{
  if( !mTheme )
    return;

  const QRect screen = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );
  //QRect fullScreen = KGlobalSettings::desktopGeometry(0L);
  KConfig *cfg = mTheme->themeConfig();
  if( !cfg )
    return;

  //if( !cfg->hasGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) ) )
  //  return;
  cfg->setGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) );

  // Overall appearance
  mBackgroundImage = cfg->readEntry( "Background Image", QString() );
  mIcon = cfg->readEntry( "User Icon", "kmenu" );
  mWelcomeText = cfg->readEntry( "Welcome Text", i18n("Welcome") );
  mUsernameText = cfg->readEntry( "Username Text", QString() );

  // If any of these are set to (0,0), then we will autoposition the text later (and it _will_
  // be centered on the screen!). The Theme may move this text however the author desires.
  QPoint absZero( 0, 0 );
  mWelcomeTextPosition  = cfg->readEntry( QString("Welcome Text Position %1").arg(screen.width()), absZero );
  mUsernameTextPosition = cfg->readEntry( QString("Username Text Position %1").arg(screen.width()), absZero );
  mActionTextPosition   = cfg->readEntry( QString("Action Text Position %1").arg(screen.width()), absZero );
  mIconPosition         = cfg->readEntry( QString("Icon Position %1").arg(screen.width()), absZero );

  // Allow the Theme to hide particular components.
  mShowWelcomeText       = cfg->readEntry( "Show Welcome Text", QVariant(true )).toBool();
  mShowWelcomeTextShadow = cfg->readEntry( "Show Welcome Shadow", QVariant(true )).toBool();
  mShowUsernameText      = cfg->readEntry( "Show Username", QVariant(true )).toBool();
  mShowActionText        = cfg->readEntry( "Show Action", QVariant(true )).toBool();
  mShowIcon              = cfg->readEntry( "Show Icon", QVariant(true )).toBool();
  mUseKdmUserIcon        = cfg->readEntry( "Use KDM User Icon", QVariant(true )).toBool();

  // Setup our fonts. There are only 3 elements which use 'em, so this is fairly
  // straightforward.
  QFont defaultFont( "Arial", 48, QFont::Bold );
  defaultFont.setItalic( true );
  QFont defaultUsernameFont( "Arial", 16, QFont::Bold );
  QFont defaultActionFont( "Arial", 12, QFont::Bold );

  mWelcomeFont       = cfg->readEntry( "Welcome Font", defaultFont );
  mWelcomeFontItalic = cfg->readEntry( "Welcome Font Italic", true );
  mUsernameFont      = cfg->readEntry( "Username Font", defaultUsernameFont );
  mActionFont        = cfg->readEntry( "Action Font", defaultActionFont );

  QColor defaultDarkColor( 3, 47, 156 );
  QColor defaultWhiteColor( Qt::white );

  mWelcomeTextColor       = cfg->readEntry( "Welcome Text Color", defaultWhiteColor );
  mWelcomeTextShadowColor = cfg->readEntry( "Welcome Shadow Color", defaultDarkColor );
  mUsernameTextColor      = cfg->readEntry( "Username Text Color", defaultWhiteColor );
  mActionTextColor        = cfg->readEntry( "Action Text Color", defaultWhiteColor );
}
