// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/*  Copyright (C) 2003 Lukas Tinkl <lukas@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ktheme.h"

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QRegExp>
#include <QTextStream>
#include <QDir>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kipc.h>
#include <klocale.h>
#include <kservice.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kstyle.h>
#include <QX11Info>

                             KTheme::KTheme( QWidget *parent, const QString & xmlFile )
                                 : m_parent(parent)
{
    QFile file( xmlFile );
    file.open( QIODevice::ReadOnly );
    m_dom.setContent( file.readAll() );
    file.close();

    //kDebug() << m_dom.toString( 2 ) << endl;

    setName( QFileInfo( file ).baseName() );
    m_kgd = KGlobal::dirs();
}

KTheme::KTheme( QWidget *parent, bool create )
    : m_parent(parent)
{
    if ( create )
    {
        m_dom = QDomDocument( "ktheme" );

        m_root = m_dom.createElement( "ktheme" );
        m_root.setAttribute( "version", SYNTAX_VERSION );
        m_dom.appendChild( m_root );

        m_general = m_dom.createElement( "general" );
        m_root.appendChild( m_general );
    }

    m_kgd = KGlobal::dirs();
}

KTheme::~KTheme()
{
}

void KTheme::setName( const QString & name )
{
    m_name = name;
}

bool KTheme::load( const KUrl & url )
{
    kDebug() << "Loading theme from URL: " << url << endl;

    QString tmpFile;
    if ( !KIO::NetAccess::download( url, tmpFile, 0L ) )
        return false;

    kDebug() << "Theme is in temp file: " << tmpFile << endl;

    // set theme's name
    setName( QFileInfo( url.fileName() ).baseName() );

    // unpack the tarball
    QString location = m_kgd->saveLocation( "themes", m_name + "/" );
    KTar tar( tmpFile );
    tar.open( QIODevice::ReadOnly );
    tar.directory()->copyTo( location );
    tar.close();

    // create the DOM
    QFile file( location + m_name + ".xml" );
    file.open( QIODevice::ReadOnly );
    m_dom.setContent( file.readAll() );
    file.close();

    // remove the temp file
    KIO::NetAccess::removeTempFile( tmpFile );

    return true;
}

QString KTheme::createYourself( bool pack )
{
    // start with empty dir for orig theme
    if ( !pack )
        KTheme::remove( name() );

    // 1. General stuff set by methods setBlah()

    // 2. Background theme
    KConfig * globalConf = KGlobal::config();

    KConfig kwinConf( "kwinrc", true );
    kwinConf.setGroup( "Desktops" );
    uint numDesktops = kwinConf.readEntry( "Number", 4 );

    KConfig desktopConf( "kdesktoprc", true );
    desktopConf.setGroup( "Background Common" );
    bool common = desktopConf.readEntry( "CommonDesktop", true );

    for ( uint i=0; i < numDesktops-1; i++ )
    {
        QDomElement desktopElem = m_dom.createElement( "desktop" );
        desktopElem.setAttribute( "number", i );
        desktopElem.setAttribute( "common", common );

        desktopConf.setGroup( "Desktop" + QString::number( i ) );

        QDomElement modeElem = m_dom.createElement( "mode" );
        modeElem.setAttribute( "id", desktopConf.readEntry( "BackgroundMode", "Flat" ) );
        desktopElem.appendChild( modeElem );

        QDomElement c1Elem = m_dom.createElement( "color1" );
        c1Elem.setAttribute( "rgb", desktopConf.readEntry( "Color1",QColor() ).name() );
        desktopElem.appendChild( c1Elem );

        QDomElement c2Elem = m_dom.createElement( "color2" );
        c2Elem.setAttribute( "rgb", desktopConf.readEntry( "Color2",QColor() ).name() );
        desktopElem.appendChild( c2Elem );

        QDomElement blendElem = m_dom.createElement( "blending" );
        blendElem.setAttribute( "mode", desktopConf.readEntry( "BlendMode", QString( "NoBlending" ) ) );
        blendElem.setAttribute( "balance", desktopConf.readEntry( "BlendBalance", QString::number( 100 ) ) );
        blendElem.setAttribute( "reverse", desktopConf.readEntry( "ReverseBlending", false ) );
        desktopElem.appendChild( blendElem );

        QDomElement patElem = m_dom.createElement( "pattern" );
        patElem.setAttribute( "name", desktopConf.readEntry( "Pattern" ) );
        desktopElem.appendChild( patElem );

        QDomElement wallElem = m_dom.createElement( "wallpaper" );
        wallElem.setAttribute( "url", processFilePath( "desktop", desktopConf.readPathEntry( "Wallpaper" ) ) );
        wallElem.setAttribute( "mode", desktopConf.readEntry( "WallpaperMode" ) );
        desktopElem.appendChild( wallElem );

        // TODO handle multi wallpapers (aka slideshow)

        m_root.appendChild( desktopElem );

        if ( common )           // generate only one node
            break;
    }

    // 11. Screensaver
    desktopConf.setGroup( "ScreenSaver" );
    QDomElement saverElem = m_dom.createElement( "screensaver" );
    saverElem.setAttribute( "name", desktopConf.readEntry( "Saver" ) );
    m_root.appendChild( saverElem );

    // 3. Icons
    globalConf->setGroup( "Icons" );
    QDomElement iconElem = m_dom.createElement( "icons" );
    iconElem.setAttribute( "name", globalConf->readEntry( "Theme",KIconTheme::current() ) );
    createIconElems( "DesktopIcons", "desktop", iconElem, globalConf );
    createIconElems( "MainToolbarIcons", "mainToolbar", iconElem, globalConf );
    createIconElems( "PanelIcons", "panel", iconElem, globalConf );
    createIconElems( "SmallIcons", "small", iconElem, globalConf );
    createIconElems( "ToolbarIcons", "toolbar", iconElem, globalConf );
    m_root.appendChild( iconElem );

    // 4. Sounds
    // 4.1 Global sounds
    KConfig * soundConf = new KConfig( "knotify.eventsrc", true );
    QStringList stdEvents;
    stdEvents << "cannotopenfile" << "catastrophe" << "exitkde" << "fatalerror"
              << "notification" << "printerror" << "startkde" << "warning"
              << "messageCritical" << "messageInformation" << "messageWarning"
              << "messageboxQuestion";

    // 4.2 KWin sounds
    KConfig * kwinSoundConf = new KConfig( "kwin.eventsrc", true );
    QStringList kwinEvents;
    kwinEvents << "activate" << "close" << "delete" <<
        "desktop1" << "desktop2" << "desktop3" << "desktop4" <<
        "desktop5" << "desktop6" << "desktop7" << "desktop8" <<
        "maximize" << "minimize" << "moveend" << "movestart" <<
        "new" << "not_on_all_desktops" << "on_all_desktops" <<
        "resizeend" << "resizestart" << "shadedown" << "shadeup" <<
        "transdelete" << "transnew" << "unmaximize" << "unminimize";

    QDomElement soundsElem = m_dom.createElement( "sounds" );
    createSoundList( stdEvents, "global", soundsElem, soundConf );
    createSoundList( kwinEvents, "kwin", soundsElem, kwinSoundConf );
    m_root.appendChild( soundsElem );
    delete soundConf;
    delete kwinSoundConf;


    // 5. Colors
    QDomElement colorsElem = m_dom.createElement( "colors" );
    globalConf->setGroup( "KDE" );
    colorsElem.setAttribute( "contrast", globalConf->readEntry( "contrast", 7 ) );
    QStringList stdColors;
    stdColors << "background" << "selectBackground" << "foreground" << "windowForeground"
              << "windowBackground" << "selectForeground" << "buttonBackground"
              << "buttonForeground" << "linkColor" << "visitedLinkColor" << "alternateBackground";

    globalConf->setGroup( "General" );
    for ( QStringList::Iterator it = stdColors.begin(); it != stdColors.end(); ++it )
        createColorElem( ( *it ), "global", colorsElem, globalConf );

    QStringList kwinColors;
    kwinColors << "activeForeground" << "inactiveBackground" << "inactiveBlend" << "activeBackground"
               << "activeBlend" << "inactiveForeground" << "activeTitleBtnBg" << "inactiveTitleBtnBg"
               << "frame" << "inactiveFrame" << "handle" << "inactiveHandle";
    globalConf->setGroup( "WM" );
    for ( QStringList::Iterator it = kwinColors.begin(); it != kwinColors.end(); ++it )
        createColorElem( ( *it ), "kwin", colorsElem, globalConf );

    m_root.appendChild( colorsElem );

    // 6. Cursors
    KConfig* mouseConf = new KConfig( "kcminputrc", true );
    mouseConf->setGroup( "Mouse" );
    QDomElement cursorsElem = m_dom.createElement( "cursors" );
    cursorsElem.setAttribute( "name", mouseConf->readEntry( "cursorTheme" ) );
    m_root.appendChild( cursorsElem );
    delete mouseConf;
    // TODO copy the cursor theme?

    // 7. KWin
    kwinConf.setGroup( "Style" );
    QDomElement wmElem = m_dom.createElement( "wm" );
    wmElem.setAttribute( "name", kwinConf.readEntry( "PluginLib" ) );
    wmElem.setAttribute( "type", "builtin" );    // TODO support pixmap themes when the kwin client gets ported
    if ( kwinConf.readEntry( "CustomButtonPositions" , false)  )
    {
        QDomElement buttonsElem = m_dom.createElement( "buttons" );
        buttonsElem.setAttribute( "left", kwinConf.readEntry( "ButtonsOnLeft" ) );
        buttonsElem.setAttribute( "right", kwinConf.readEntry( "ButtonsOnRight" ) );
        wmElem.appendChild( buttonsElem );
    }
    QDomElement borderElem = m_dom.createElement( "border" );
    borderElem.setAttribute( "size", kwinConf.readEntry( "BorderSize", 1 ) );
    wmElem.appendChild( borderElem );
    m_root.appendChild( wmElem );

    // 8. Konqueror
    KConfig konqConf( "konquerorrc", true );
    konqConf.setGroup( "Settings" );
    QDomElement konqElem = m_dom.createElement( "konqueror" );
    QDomElement konqWallElem = m_dom.createElement( "wallpaper" );
    QString bgImagePath = konqConf.readPathEntry( "BgImage" );
    konqWallElem.setAttribute( "url", processFilePath( "konqueror", bgImagePath ) );
    konqElem.appendChild( konqWallElem );
    QDomElement konqBgColorElem = m_dom.createElement( "bgcolor" );
    konqBgColorElem.setAttribute( "rgb", konqConf.readEntry( "BgColor",QColor() ).name() );
    konqElem.appendChild( konqBgColorElem );
    m_root.appendChild( konqElem );

    // 9. Kicker (aka KDE Panel)
    KConfig kickerConf( "kickerrc", true );
    kickerConf.setGroup( "General" );

    QDomElement panelElem = m_dom.createElement( "panel" );

    if ( kickerConf.readEntry( "UseBackgroundTheme" , false) )
    {
        QDomElement backElem = m_dom.createElement( "background" );
        QString kbgPath = kickerConf.readPathEntry( "BackgroundTheme" );
        backElem.setAttribute( "url", processFilePath( "panel", kbgPath ) );
        backElem.setAttribute( "colorize", kickerConf.readEntry( "ColorizeBackground" , false) );
        panelElem.appendChild( backElem );
    }

    QDomElement transElem = m_dom.createElement( "transparent" );
    transElem.setAttribute( "value", kickerConf.readEntry( "Transparent" , false) );
    panelElem.appendChild( transElem );
    m_root.appendChild( panelElem );

    // 10. Widget style
    globalConf->setGroup( "General" );
    QDomElement widgetsElem = m_dom.createElement( "widgets" );
#warning "qt4: FIXME KStyle::defaultStyle();";
    widgetsElem.setAttribute( "name", globalConf->readEntry( "widgetStyle"/*,KStyle::defaultStyle()*/  ) );
    m_root.appendChild( widgetsElem );

    // 12.
    QDomElement fontsElem = m_dom.createElement( "fonts" );
    QStringList fonts;
    fonts   << "General"    << "font"
            << "General"    << "fixed"
            << "General"    << "toolBarFont"
            << "General"    << "menuFont"
            << "WM"         << "activeFont"
            << "General"    << "taskbarFont"
            << "FMSettings" << "StandardFont";

    for ( QStringList::Iterator it = fonts.begin(); it != fonts.end(); ++it ) {
        QString group = *it; ++it;
        QString key   = *it;
        QString value;

        if ( group == "FMSettings" ) {
            desktopConf.setGroup( group );
            value = desktopConf.readEntry( key, QString() );
        }
        else {
            globalConf->setGroup( group );
            value = globalConf->readEntry( key, QString() );
        }
        QDomElement fontElem = m_dom.createElement( key );
        fontElem.setAttribute( "object", group );
        fontElem.setAttribute( "value", value );
        fontsElem.appendChild( fontElem );
    }
    m_root.appendChild( fontsElem );

    // Save the XML
    QFile file( m_kgd->saveLocation( "themes", m_name + "/" ) + m_name + ".xml" );
    if ( file.open( QIODevice::WriteOnly ) ) {
        QTextStream stream( &file );
        m_dom.save( stream, 2 );
        file.close();
    }

    QString result;
    if ( pack )
    {
        // Pack the whole theme
        KTar tar( m_kgd->saveLocation( "themes" ) + m_name + ".kth", "application/x-gzip" );
        tar.open( QIODevice::WriteOnly );

        kDebug() << "Packing everything under: " << m_kgd->saveLocation( "themes", m_name + "/" ) << endl;

        if ( tar.addLocalDirectory( m_kgd->saveLocation( "themes", m_name + "/" ), QString() ) )
            result = tar.fileName();

        tar.close();
    }

    //kDebug() << m_dom.toString( 2 ) << endl;

    return result;
}

void KTheme::apply()
{
    kDebug() << "Going to apply theme: " << m_name << endl;

    QString themeDir = m_kgd->findResourceDir( "themes", m_name + "/" + m_name + ".xml") + m_name + "/";
    kDebug() << "Theme dir: " << themeDir << endl;

    // 2. Background theme

    QDomNodeList desktopList = m_dom.elementsByTagName( "desktop" );
    KConfig desktopConf( "kdesktoprc" );
    desktopConf.setGroup( "Background Common" );

    for ( int i = 0; i <= desktopList.count(); i++ )
    {
        QDomElement desktopElem = desktopList.item( i ).toElement();
        if ( !desktopElem.isNull() )
        {
            // TODO optimize, don't write several times the common section
            bool common = static_cast<bool>( desktopElem.attribute( "common", "true" ).toUInt() );
            desktopConf.writeEntry( "CommonDesktop", common );
            desktopConf.writeEntry( "DeskNum", desktopElem.attribute( "number", "0" ).toUInt() );

            desktopConf.setGroup( QString( "Desktop%1" ).arg( i ) );
            desktopConf.writeEntry( "BackgroundMode", getProperty( desktopElem, "mode", "id" ) );
            desktopConf.writeEntry( "Color1", QColor( getProperty( desktopElem, "color1", "rgb" ) ) );
            desktopConf.writeEntry( "Color2", QColor( getProperty( desktopElem, "color2", "rgb" ) ) );
            desktopConf.writeEntry( "BlendMode", getProperty( desktopElem, "blending", "mode" ) );
            desktopConf.writeEntry( "BlendBalance", getProperty( desktopElem, "blending", "balance" ) );
            desktopConf.writeEntry( "ReverseBlending",
                                    static_cast<bool>( getProperty( desktopElem, "blending", "reverse" ).toUInt() ) );
            desktopConf.writeEntry( "Pattern", getProperty( desktopElem, "pattern", "name" ) );
            desktopConf.writeEntry( "Wallpaper",
                                    unprocessFilePath( "desktop", getProperty( desktopElem, "wallpaper", "url" ) ) );
            desktopConf.writeEntry( "WallpaperMode", getProperty( desktopElem, "wallpaper", "mode" ) );

            if ( common )
                break;          // stop here
        }
    }

    // 11. Screensaver
    QDomElement saverElem = m_dom.elementsByTagName( "screensaver" ).item( 0 ).toElement();

    if ( !saverElem.isNull() )
    {
        desktopConf.setGroup( "ScreenSaver" );
        desktopConf.writeEntry( "Saver", saverElem.attribute( "name" ) );
    }

    desktopConf.sync();         // TODO sync and signal only if <desktop> elem present
    // reconfigure kdesktop. kdesktop will notify all clients
    DCOPClient *client = kapp->dcopClient();
    if ( !client->isAttached() )
        client->attach();
	QByteArray data;
    client->send("kdesktop", "KBackgroundIface", "configure()", data);
    // FIXME Xinerama

    // 3. Icons
    QDomElement iconElem = m_dom.elementsByTagName( "icons" ).item( 0 ).toElement();
    if ( !iconElem.isNull() )
    {
        KConfig * iconConf = KGlobal::config();
        iconConf->setGroup( "Icons" );
        iconConf->writeEntry( "Theme", iconElem.attribute( "name", "crystalsvg" ), KConfigBase::Persistent|KConfigBase::Global);

        QDomNodeList iconList = iconElem.childNodes();
        for ( int i = 0; i < iconList.count(); i++ )
        {
            QDomElement iconSubElem = iconList.item( i ).toElement();
            QString object = iconSubElem.attribute( "object" );
            if ( object == "desktop" )
                iconConf->setGroup( "DesktopIcons" );
            else if ( object == "mainToolbar" )
                iconConf->setGroup( "MainToolbarIcons" );
            else if ( object == "panel" )
                iconConf->setGroup( "PanelIcons" );
            else if ( object == "small" )
                iconConf->setGroup( "SmallIcons" );
            else if ( object == "toolbar" )
                iconConf->setGroup( "ToolbarIcons" );

            QString iconName = iconSubElem.tagName();
            if ( iconName.contains( "Color" ) )
            {
                QColor iconColor = QColor( iconSubElem.attribute( "rgb" ) );
                iconConf->writeEntry( iconName, iconColor, KConfigBase::Persistent|KConfigBase::Global);
            }
            else if ( iconName.contains( "Value" ) || iconName == "Size" )
                iconConf->writeEntry( iconName, iconSubElem.attribute( "value" ).toUInt(), KConfigBase::Persistent|KConfigBase::Global);
            else if ( iconName.contains( "Effect" ) )
                iconConf->writeEntry( iconName, iconSubElem.attribute( "name" ), KConfigBase::Persistent|KConfigBase::Global);
            else
                iconConf->writeEntry( iconName, static_cast<bool>( iconSubElem.attribute( "value" ).toUInt() ), KConfigBase::Persistent|KConfigBase::Global);
        }
        iconConf->sync();

        for ( int i = 0; i < K3Icon::LastGroup; i++ )
            KIPC::sendMessageAll( KIPC::IconChanged, i );
        KService::rebuildKSycoca( m_parent );
    }

    // 4. Sounds
    QDomElement soundsElem = m_dom.elementsByTagName( "sounds" ).item( 0 ).toElement();
    if ( !soundsElem.isNull() )
    {
        KConfig soundConf( "knotify.eventsrc" );
        KConfig kwinSoundConf( "kwin.eventsrc" );
        QDomNodeList eventList = soundsElem.elementsByTagName( "event" );
        for ( int i = 0; i < eventList.count(); i++ )
        {
            QDomElement eventElem = eventList.item( i ).toElement();
            QString object = eventElem.attribute( "object" );

            if ( object == "global" )
            {
                soundConf.setGroup( eventElem.attribute( "name" ) );
                soundConf.writeEntry( "soundfile", unprocessFilePath( "sounds", eventElem.attribute( "url" ) ) );
                soundConf.writeEntry( "presentation", soundConf.readEntry( "presentation" ,0) | 1 );
            }
            else if ( object == "kwin" )
            {
                kwinSoundConf.setGroup( eventElem.attribute( "name" ) );
                kwinSoundConf.writeEntry( "soundfile", unprocessFilePath( "sounds", eventElem.attribute( "url" ) ) );
                kwinSoundConf.writeEntry( "presentation", soundConf.readEntry( "presentation",0 ) | 1 );
            }
        }

        soundConf.sync();
        kwinSoundConf.sync();
		QByteArray data;
        client->send("knotify", "", "reconfigure()", data);
        // TODO signal kwin sounds change?
    }

    // 5. Colors
    QDomElement colorsElem = m_dom.elementsByTagName( "colors" ).item( 0 ).toElement();

    if ( !colorsElem.isNull() )
    {
        QDomNodeList colorList = colorsElem.childNodes();
        KConfig * colorConf = KGlobal::config();

        QString sCurrentScheme = locateLocal("data", "kdisplay/color-schemes/thememgr.kcsrc");
        KSimpleConfig *colorScheme = new KSimpleConfig( sCurrentScheme );
        colorScheme->setGroup("Color Scheme" );

        for ( int i = 0; i < colorList.count(); i++ )
        {
            QDomElement colorElem = colorList.item( i ).toElement();
            QString object = colorElem.attribute( "object" );
            if ( object == "global" )
                colorConf->setGroup( "General" );
            else if ( object == "kwin" )
                colorConf->setGroup( "WM" );

            QString colName = colorElem.tagName();
            QColor curColor = QColor( colorElem.attribute( "rgb" ) );
            colorConf->writeEntry( colName, curColor, KConfigBase::Persistent|KConfigBase::Global); // kdeglobals
            colorScheme->writeEntry( colName, curColor ); // thememgr.kcsrc
        }

        colorConf->setGroup( "KDE" );
        colorConf->writeEntry( "colorScheme", "thememgr.kcsrc", KConfigBase::Persistent|KConfigBase::Global);
        colorConf->writeEntry( "contrast", colorsElem.attribute( "contrast", "7" ), KConfigBase::Persistent|KConfigBase::Global);
        colorScheme->writeEntry( "contrast", colorsElem.attribute( "contrast", "7" ) );
        colorConf->sync();
        delete colorScheme;

        KIPC::sendMessageAll( KIPC::PaletteChanged );
    }

    // 6.Cursors
    QDomElement cursorsElem = m_dom.elementsByTagName( "cursors" ).item( 0 ).toElement();

    if ( !cursorsElem.isNull() )
    {
        KConfig mouseConf( "kcminputrc" );
        mouseConf.setGroup( "Mouse" );
        mouseConf.writeEntry( "cursorTheme", cursorsElem.attribute( "name" ));
        // FIXME is there a way to notify KDE of cursor changes?
    }

    // 7. KWin
    QDomElement wmElem = m_dom.elementsByTagName( "wm" ).item( 0 ).toElement();

    if ( !wmElem.isNull() )
    {
        KConfig kwinConf( "kwinrc" );
        kwinConf.setGroup( "Style" );
        QString type = wmElem.attribute( "type" );
        if ( type == "builtin" )
            kwinConf.writeEntry( "PluginLib", wmElem.attribute( "name" ) );
        //else // TODO support custom themes
        QDomNodeList buttons = wmElem.elementsByTagName ("buttons");
        if ( buttons.count() > 0 )
        {
            kwinConf.writeEntry( "CustomButtonPositions", true );
            kwinConf.writeEntry( "ButtonsOnLeft", getProperty( wmElem, "buttons", "left" ) );
            kwinConf.writeEntry( "ButtonsOnRight", getProperty( wmElem, "buttons", "right" ) );
        }
        else
        {
            kwinConf.writeEntry( "CustomButtonPositions", false );
        }
        kwinConf.writeEntry( "BorderSize", getProperty( wmElem, "border", "size" ) );

        kwinConf.sync();
        client->send( DCOPCString("kwin"), DCOPCString(""), DCOPCString("reconfigure()"), DCOPCString("") );
    }

    // 8. Konqueror
    QDomElement konqElem = m_dom.elementsByTagName( "konqueror" ).item( 0 ).toElement();

    if ( !konqElem.isNull() )
    {
        KConfig konqConf( "konquerorrc" );
        konqConf.setGroup( "Settings" );
        konqConf.writeEntry( "BgImage", unprocessFilePath( "konqueror", getProperty( konqElem, "wallpaper", "url" ) ) );
        konqConf.writeEntry( "BgColor", QColor( getProperty( konqElem, "bgcolor", "rgb" ) ) );

        konqConf.sync();
		QByteArray data;
        client->send("konqueror*", "KonquerorIface", "reparseConfiguration()", data); // FIXME seems not to work :(
    }

    // 9. Kicker
    QDomElement panelElem = m_dom.elementsByTagName( "panel" ).item( 0 ).toElement();

    if ( !panelElem.isNull() )
    {
        KConfig kickerConf( "kickerrc" );
        kickerConf.setGroup( "General" );
        QString kickerBgUrl = getProperty( panelElem, "background", "url" );
        if ( !kickerBgUrl.isEmpty() )
        {
            kickerConf.writeEntry( "UseBackgroundTheme", true );
            kickerConf.writeEntry( "BackgroundTheme", unprocessFilePath( "panel", kickerBgUrl ) );
            kickerConf.writeEntry( "ColorizeBackground",
                                   static_cast<bool>( getProperty( panelElem, "background", "colorize" ).toUInt() ) );
        }
        kickerConf.writeEntry( "Transparent",
                               static_cast<bool>( getProperty( panelElem, "transparent", "value" ).toUInt() ) );

        kickerConf.sync();
		QByteArray data;
        client->send("kicker", "Panel", "configure()", data);
    }

    // 10. Widget style
    QDomElement widgetsElem = m_dom.elementsByTagName( "widgets" ).item( 0 ).toElement();

    if ( !widgetsElem.isNull() )
    {
        KConfig * widgetConf = KGlobal::config();
        widgetConf->setGroup( "General" );
        widgetConf->writeEntry( "widgetStyle", widgetsElem.attribute( "name" ), KConfigBase::Persistent|KConfigBase::Global);
        widgetConf->sync();
        KIPC::sendMessageAll( KIPC::StyleChanged );
    }

    // 12. Fonts
    QDomElement fontsElem = m_dom.elementsByTagName( "fonts" ).item( 0 ).toElement();
    if ( !fontsElem.isNull() )
    {
        KConfig * fontsConf = KGlobal::config();
        KConfig * kde1xConf = new KSimpleConfig( QDir::homePath() + "/.kderc" );
        kde1xConf->setGroup( "General" );

        QDomNodeList fontList = fontsElem.childNodes();
        for ( int i = 0; i < fontList.count(); i++ )
        {
            QDomElement fontElem = fontList.item( i ).toElement();
            QString fontName  = fontElem.tagName();
            QString fontValue = fontElem.attribute( "value" );
            QString fontObject = fontElem.attribute( "object" );

            if ( fontObject == "FMSettings" ) {
                desktopConf.setGroup( fontObject );
                desktopConf.writeEntry( fontName, fontValue, KConfigBase::Persistent|KConfigBase::Global);
                desktopConf.sync();
            }
            else {
                fontsConf->setGroup( fontObject );
                fontsConf->writeEntry( fontName, fontValue, KConfigBase::Persistent|KConfigBase::Global);
            }
            kde1xConf->writeEntry( fontName, fontValue, KConfigBase::Persistent|KConfigBase::Global);
        }

        fontsConf->sync();
        kde1xConf->sync();
        KIPC::sendMessageAll( KIPC::FontChanged );
    }

}

bool KTheme::remove( const QString & name )
{
    kDebug() << "Going to remove theme: " << name << endl;
    return KIO::NetAccess::del( KGlobal::dirs()->saveLocation( "themes", name + "/" ), 0L );
}

void KTheme::setProperty( const QString & name, const QString & value, QDomElement parent )
{
    QDomElement tmp = m_dom.createElement( name );
    tmp.setAttribute( "value", value );
    parent.appendChild( tmp );
}

QString KTheme::getProperty( const QString & name ) const
{
    QDomNodeList _list = m_dom.elementsByTagName( name );
    if ( _list.count() != 0 )
        return _list.item( 0 ).toElement().attribute( "value" );
    else
    {
        kWarning() << "Found no such property: " << name << endl;
        return QString();
    }
}

QString KTheme::getProperty( QDomElement parent, const QString & tag,
                             const QString & attr ) const
{
    QDomNodeList _list = parent.elementsByTagName( tag );

    if ( _list.count() != 0 )
        return _list.item( 0 ).toElement().attribute( attr );
    else
    {
        kWarning() << QString( "No such property found: %1->%2->%3" )
            .arg( parent.tagName() ).arg( tag ).arg( attr ) << endl;
        return QString();
    }
}

void KTheme::createIconElems( const QString & group, const QString & object,
                              QDomElement parent, KConfig * cfg )
{
    cfg->setGroup( group );
    QStringList elemNames;
    elemNames << "Animated" << "DoublePixels" << "Size"
              << "ActiveColor" << "ActiveColor2" << "ActiveEffect"
              << "ActiveSemiTransparent" << "ActiveValue"
              << "DefaultColor" << "DefaultColor2" << "DefaultEffect"
              << "DefaultSemiTransparent" << "DefaultValue"
              << "DisabledColor" << "DisabledColor2" << "DisabledEffect"
              << "DisabledSemiTransparent" << "DisabledValue";
    for ( QStringList::ConstIterator it = elemNames.begin(); it != elemNames.end(); ++it ) {
        if ( (*it).contains( "Color" ) )
            createColorElem( *it, object, parent, cfg );
        else
        {
            QDomElement tmpCol = m_dom.createElement( *it );
            tmpCol.setAttribute( "object", object );

            if ( (*it).contains( "Value" ) || *it == "Size" )
                tmpCol.setAttribute( "value", cfg->readEntry( *it, 1 ) );
	    else if ( (*it).contains( "DisabledEffect" ) ) 
		tmpCol.setAttribute( "name", cfg->readEntry( *it, QString("togray") ) ); 
            else if ( (*it).contains( "Effect" ) )
                tmpCol.setAttribute( "name", cfg->readEntry( *it, QString("none") ) );
            else
                tmpCol.setAttribute( "value", cfg->readEntry( *it, false ) );
            parent.appendChild( tmpCol );
        }
    }
}

void KTheme::createColorElem( const QString & name, const QString & object,
                              QDomElement parent, KConfig * cfg )
{
    QColor color = cfg->readEntry( name,QColor() );
    if ( color.isValid() )
    {
        QDomElement tmpCol = m_dom.createElement( name );
        tmpCol.setAttribute( "rgb", color.name() );
        tmpCol.setAttribute( "object", object );
        parent.appendChild( tmpCol );
    }
}

void KTheme::createSoundList( const QStringList & events, const QString & object,
                              QDomElement parent, KConfig * cfg )
{
    for ( QStringList::ConstIterator it = events.begin(); it != events.end(); ++it )
    {
        QString group = ( *it );
        if ( cfg->hasGroup( group ) )
        {
            cfg->setGroup( group );
            QString soundURL = cfg->readPathEntry( "soundfile" );
            int pres = cfg->readEntry( "presentation", 0 );
            if ( !soundURL.isEmpty() && ( ( pres & 1 ) == 1 ) )
            {
                QDomElement eventElem = m_dom.createElement( "event" );
                eventElem.setAttribute( "object", object );
                eventElem.setAttribute( "name", group );
                eventElem.setAttribute( "url", processFilePath( "sounds", soundURL ) );
                parent.appendChild( eventElem );
            }
        }
    }
}

QString KTheme::processFilePath( const QString & section, const QString & path )
{
    QFileInfo fi( path );

    if ( fi.isRelative() )
        fi.setFile( findResource( section, path ) );

    kDebug() << "Processing file: " << fi.absoluteFilePath() << ", " << fi.fileName() << endl;

    if ( section == "desktop" )
    {
        if ( copyFile( fi.absoluteFilePath(), m_kgd->saveLocation( "themes", m_name + "/wallpapers/desktop/" ) + "/" + fi.fileName() ) )
            return "theme:/wallpapers/desktop/" + fi.fileName();
    }
    else if ( section == "sounds" )
    {
        if ( copyFile( fi.absoluteFilePath(), m_kgd->saveLocation( "themes", m_name + "/sounds/" ) + "/" + fi.fileName() ) )
            return "theme:/sounds/" + fi.fileName();
    }
    else if ( section == "konqueror" )
    {
        if ( copyFile( fi.absoluteFilePath(), m_kgd->saveLocation( "themes", m_name + "/wallpapers/konqueror/" ) + "/" + fi.fileName() ) )
            return "theme:/wallpapers/konqueror/" + fi.fileName();
    }
    else if ( section == "panel" )
    {
        if ( copyFile( fi.absoluteFilePath(), m_kgd->saveLocation( "themes", m_name + "/wallpapers/panel/" ) + "/" + fi.fileName() ) )
            return "theme:/wallpapers/panel/" + fi.fileName();
    }
    else
        kWarning() << "Unsupported theme resource type" << endl;

    return QString();       // an error occured or the resource doesn't exist
}

QString KTheme::unprocessFilePath( const QString & section, QString path )
{
    if ( path.startsWith( "theme:/" ) )
        return path.replace( QRegExp( "^theme:/" ), m_kgd->findResourceDir( "themes", m_name + "/" + m_name + ".xml") + m_name + "/" );

    if ( QFile::exists( path ) )
        return path;
    else  // try to find it in the system
        return findResource( section, path );
}

void KTheme::setAuthor( const QString & author )
{
    setProperty( "author", author, m_general );
}

void KTheme::setEmail( const QString & email )
{
    setProperty( "email", email, m_general );
}

void KTheme::setHomepage( const QString & homepage )
{
    setProperty( "homepage", homepage, m_general );
}

void KTheme::setComment( const QString & comment )
{
    setProperty( "comment", comment, m_general );
}

void KTheme::setVersion( const QString & version )
{
    setProperty( "version", version, m_general );
}

void KTheme::addPreview()
{
    QString file = m_kgd->saveLocation( "themes", m_name + "/" ) + m_name + ".preview.png";
    kDebug() << "Adding preview: " << file << endl;
    QPixmap snapshot = QPixmap::grabWindow( QX11Info::appRootWindow() );
    snapshot.save( file, "PNG" );
}

bool KTheme::copyFile( const QString & from, const QString & to )
{
    // we overwrite b/c of restoring the "original" theme
    return KIO::NetAccess::file_copy( from, to, -1, true /*overwrite*/ );
}

QString KTheme::findResource( const QString & section, const QString & path )
{
    if ( section == "desktop" )
        return m_kgd->findResource( "wallpaper", path );
    else if ( section == "sounds" )
        return m_kgd->findResource( "sound", path );
    else if ( section == "konqueror" )
        return m_kgd->findResource( "data", "konqueror/tiles/" + path );
    else if ( section == "panel" )
        return m_kgd->findResource( "data", "kicker/wallpapers/" + path );
    else
    {
        kWarning() << "Requested unknown resource: " << section << endl;
        return QString();
    }
}
