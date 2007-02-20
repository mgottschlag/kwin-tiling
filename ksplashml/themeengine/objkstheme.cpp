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
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <QColor>
#include <QCursor>
#include <qdesktopwidget.h>
#include <QFont>
#include <QPixmap>
#include <QRect>
#include <QString>

#include "objkstheme.h"
#include "objkstheme.moc"

ObjKsTheme::ObjKsTheme( const QString& theme )
  :mActiveTheme (theme), mThemeDir("/"), mThemeConfig (0L), mThemePrefix( "Themes/" ), d(0)
{
  // Get Xinerama config.
  KConfigGroup config(KGlobal::config(), "Xinerama");
  QDesktopWidget *desktop = kapp->desktop();
  mXineramaScreen = config.readEntry("KSplashScreen", desktop->primaryScreen());

  // For Xinerama, let's put the mouse on the first head.  Otherwise it could appear anywhere!
  if (desktop->isVirtualDesktop() && mXineramaScreen != -2)
  {
    QRect rect = desktop->screenGeometry( mXineramaScreen );
    if (!rect.contains(QCursor::pos()))
      QCursor::setPos(rect.center());
  }

  // Does the active theme exist?
  if( !loadThemeRc( mActiveTheme, false ) )
    if( !loadLocalConfig( mActiveTheme, false ) )
      if( !loadThemeRc( "Default", false ) )
        loadLocalConfig( "Default", true ); //force: we need some defaults
  loadCmdLineArgs(KCmdLineArgs::parsedArgs());
  mThemePrefix += ( mActiveTheme + '/' );
}

ObjKsTheme::~ObjKsTheme()
{
}

bool ObjKsTheme::loadThemeRc( const QString& activeTheme, bool force )
{
  //kDebug() << "ObjKsTheme::loadThemeRc: " << activeTheme << endl;
  QString prefix("Themes/");
  QString themeFile;
  KConfig *cf = 0L;

  // Try our best to find a theme file.
  themeFile = KStandardDirs::locate( "appdata", prefix + activeTheme + '/' + QString("Theme.rc") );
  themeFile = themeFile.isEmpty() ? KStandardDirs::locate("appdata",prefix+activeTheme+'/'+QString("Theme.RC")):themeFile;
  themeFile = themeFile.isEmpty() ? KStandardDirs::locate("appdata",prefix+activeTheme+'/'+QString("theme.rc")):themeFile;
  themeFile = themeFile.isEmpty() ? KStandardDirs::locate("appdata",prefix+activeTheme+'/'+activeTheme+QString(".rc")):themeFile;

  if( !themeFile.isEmpty() )
     cf = new KConfig( themeFile );

  if( cf )
  {
    mActiveTheme = activeTheme;
    mThemeDir = prefix + activeTheme+'/';
    if( loadKConfig( cf, activeTheme, force ) )
    {
      mThemeConfig = cf;
      return true;
    }
    else
      delete cf;
  }
  return false;
}

bool ObjKsTheme::loadLocalConfig( const QString& activeTheme, bool force )
{
  //kDebug() << "ObjKsTheme::loadLocalConfig" << endl;
  KSharedConfig::Ptr cfg = KGlobal::config();
  return( loadKConfig( cfg.data(), activeTheme, force ) );
}

// ObjKsConfig::loadKConfig(): Load our settings from a KConfig object.
bool ObjKsTheme::loadKConfig( KConfig *cfg, const QString& activeTheme, bool force )
{
  //kDebug() << "ObjKsTheme::loadKConfig" << endl;
  if( !cfg )
    return false;

  // Themes are always stored in the group [KSplash Theme: ThemeName],
  // and ThemeName should always be the same name as the themedir, if any.
  // If we can't find this theme group, then we can't load.
  if( !cfg->hasGroup( QString("KSplash Theme: %1").arg(activeTheme) ) && !force )
    return false;

  cfg->setGroup( QString("KSplash Theme: %1").arg(activeTheme) );
  mThemeConfig = cfg;

  mThemeEngine = cfg->readEntry( "Engine", "Default" );

  m_icons.clear();
  m_icons.append( cfg->readEntry( "Icon1", "filetypes" ) );
  m_icons.append( cfg->readEntry( "Icon2", "exec" ) );
  m_icons.append( cfg->readEntry( "Icon3", "key_bindings" ) );
  m_icons.append( cfg->readEntry( "Icon4", "window_list" ) );
  m_icons.append( cfg->readEntry( "Icon5", "desktop" ) );
  m_icons.append( cfg->readEntry( "Icon6", "style" ) );
  m_icons.append( cfg->readEntry( "Icon7", "kcmsystem" ) );
  m_icons.append( cfg->readEntry( "Icon8", "go" ) );

  m_text.clear();
  m_text.append( cfg->readEntry( "Message1", i18n("Setting up interprocess communication") ) );
  m_text.append( cfg->readEntry( "Message2", i18n("Initializing system services") ) );
  m_text.append( cfg->readEntry( "Message3", i18n("Initializing peripherals") ) );
  m_text.append( cfg->readEntry( "Message4", i18n("Loading the window manager") ) );
  m_text.append( cfg->readEntry( "Message5", i18n("Loading the desktop") ) );
  m_text.append( cfg->readEntry( "Message6", i18n("Loading the panel") ) );
  m_text.append( cfg->readEntry( "Message7", i18n("Restoring session") ) );
  m_text.append( cfg->readEntry( "Message8", i18n("KDE is up and running") ) );

  return true;
}

/*
 * ObjKsTheme::loadCmdLineArgs(): Handle any overrides which the user might have
 * specified.
 */
void ObjKsTheme::loadCmdLineArgs( KCmdLineArgs *args )
{

  mManagedMode = args->isSet( "managed" );
  mTesting = args->isSet("test");
  mLoColor = ( QPixmap::defaultDepth() <= 8 );
  QString theme = args->getOption( "theme" );
  if( theme != mActiveTheme && !theme.isNull() )
    if( loadThemeRc( theme, false ) )
      mActiveTheme = theme;
  //args->clear();
}

QString ObjKsTheme::locateThemeData( const QString &resource )
{
  if ( !mLoColor )
    return KStandardDirs::locate( "appdata", mThemePrefix+resource );
  else
  {
    QString res = KStandardDirs::locate( "appdata", mThemePrefix+"locolor/"+resource );
    if ( res.isEmpty() )
      res = KStandardDirs::locate( "appdata", mThemePrefix+resource );
    return res;
  }
}
