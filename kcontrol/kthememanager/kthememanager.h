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

#ifndef KTHEMEMANAGER_H
#define KTHEMEMANAGER_H

#include <kcmodule.h>
#include <krun.h>
#include <kservice.h>
#include <kurl.h>

#include "kthemedlg.h"
#include "ktheme.h"

class QString;

class KAboutData;

#define ORIGINAL_THEME "original" // no i18n() here!!!

/*
class KIconViewItem;

class KThemeDetailsItem: public KIconViewItem
{
public:
    KThemeDetailsItem( KIconView * parent, const QString & text, const QPixmap & icon, const QString & execString )
        : KIconViewItem( parent, text, icon ) { m_exec = execString; }
    virtual ~KThemeDetailsItem() { };

    void exec() {
        ( void ) new KRun( m_exec );
    }
private:
    QString m_exec;
};
*/

/**
 *
 * This is the for KControl config module for installing,
 * creating and removing visual themes.
 *
 * @brief The Theme Manager config module.
 * @author Lukas Tinkl <lukas@kde.org>
 */
class kthememanager: public KCModule
{
    Q_OBJECT
public:
    kthememanager( QWidget *parent=0, const char *name=0 );
    virtual ~kthememanager();

    /**
     * Called on module startup
     */
    virtual void load();
    /**
     * Called when applying the changes
     */
    virtual void save();
    /**
     * Called when the user requests the default values
     */
    virtual void defaults();

protected:
    void dragEnterEvent ( QDragEnterEvent * ev );
    void dropEvent ( QDropEvent * ev );

signals:
    /**
     * Emitted when some @p urls are dropped onto the kcm
     */
    void filesDropped(const KURL::List &urls);

private slots:
    /**
     * Install a theme from a tarball (*.kth)
     */
    void slotInstallTheme();

    /**
     * Remove an installed theme
     */
    void slotRemoveTheme();

    /**
     * Create a new theme
     */
    void slotCreateTheme();

    /**
     * Update the theme's info and preview
     */
    void slotThemeChanged( Q3ListViewItem * item );

    /**
     * Invoked when one drag and drops @p urls onto the kcm
     * @see signal filesDropped
     */
    void slotFilesDropped( const KURL::List & urls );
    void updateButton();

private:
    /**
     * List themes available in the system and insert them into the listview.
     */
    void listThemes();

    /**
     * Performs the actual theme installation.
     */
    void addNewTheme( const KURL & url );

    /**
     * Perform internal initialization of paths.
     */
    void init();

    /**
     * Try to find out whether a theme is installed and get its version number
     * @param themeName The theme name
     * @return The theme's version number or -1 if not installed
     */
    static float getThemeVersion( const QString & themeName );

    void queryLNFModules();

    /**
     * Updates the preview widget
     */
    void updatePreview( const QString & pixFile );
    bool themeExist(const QString &_themeName);
    KThemeDlg * dlg;

    KTheme * m_theme;
    KTheme * m_origTheme;
};

#endif
