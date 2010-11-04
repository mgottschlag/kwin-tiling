/*
 * KCMDesktopTheme
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 by Davide Bettio <davide.bettio@kdemail.net>

 * Portions Copyright (C) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 * Portions Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 * Portions Copyright (C) 2008 by Petri Damsten <damu@iki.fi>
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmdesktoptheme.h"

#include "thememodel.h"


#include <kaboutdata.h>
#include <kautostart.h>
#include <KStandardDirs>
#include <knewstuff3/downloaddialog.h>

#include <Plasma/Theme>

/**** DLL Interface for kcontrol ****/

#include <kpluginfactory.h>
#include <kpluginloader.h>

K_PLUGIN_FACTORY(KCMDesktopThemeFactory, registerPlugin<KCMDesktopTheme>();)
K_EXPORT_PLUGIN(KCMDesktopThemeFactory("kcmdesktoptheme","kcm_desktopthemedetails"))


KCMDesktopTheme::KCMDesktopTheme( QWidget* parent, const QVariantList& )
    : KCModule( KCMDesktopThemeFactory::componentData(), parent )
{
    setQuickHelp( i18n("<h1>Desktop Theme</h1>"
            "This module allows you to modify the visual appearance "
            "of the desktop."));

    setupUi(this);

    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;

    KAutostart plasmaNetbookAutoStart("plasma-netbook");
    m_isNetbook = plasmaNetbookAutoStart.autostarts();

    KGlobal::dirs()->addResourceType("themes", "data", "kstyle/themes");

    KAboutData *about =
        new KAboutData( I18N_NOOP("KCMDesktopTheme"), 0,
                        ki18n("KDE Desktop Theme Module"),
                        0, KLocalizedString(), KAboutData::License_GPL,
                        ki18n("(c) 2002 Karol Szwed, Daniel Molkentin"));

    about->addAuthor(ki18n("Karol Szwed"), KLocalizedString(), "gallium@kde.org");
    about->addAuthor(ki18n("Daniel Molkentin"), KLocalizedString(), "molkentin@kde.org");
    about->addAuthor(ki18n("Ralf Nolden"), KLocalizedString(), "nolden@kde.org");
    setAboutData( about );

    m_newThemeButton->setIcon(KIcon("get-hot-new-stuff"));

    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme));
    m_theme->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_detailsWidget, SIGNAL(changed()), this, SLOT(detailChanged()));

    connect(m_theme->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setDesktopThemeDirty()));
    connect(m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));
}


KCMDesktopTheme::~KCMDesktopTheme()
{
}

void KCMDesktopTheme::load()
{
    KConfig config( "kdeglobals", KConfig::FullConfig );

    loadDesktopTheme();

    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;

    emit changed( false );
}


void KCMDesktopTheme::save()
{
    // Don't do anything if we don't need to.
    if ( !( m_bDesktopThemeDirty) && !(m_bDetailsDirty) )
        return;

    //Desktop theme
    if ( m_bDesktopThemeDirty )
    {
        QString theme = m_themeModel->data(m_theme->currentIndex(), ThemeModel::PackageNameRole).toString();
        if (m_isNetbook) {
            KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
            cg.writeEntry("name", theme);
        } else {
            Plasma::Theme::defaultTheme()->setThemeName(theme);
        }
    }

    if (m_bDetailsDirty)
    {
        m_detailsWidget->save();
    }

    // Clean up
    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;
    emit changed( false );
}

void KCMDesktopTheme::defaults()
{
    // TODO: reset back to default theme?
}

void KCMDesktopTheme::setDesktopThemeDirty()
{
    m_bDesktopThemeDirty = true;
    emit changed(true);
}

void KCMDesktopTheme::getNewThemes()
{
    KNS3::DownloadDialog dialog("plasma-themes.knsrc", this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();

    if (entries.size() > 0) {
        loadDesktopTheme();
    }
}

void KCMDesktopTheme::loadDesktopTheme()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_themeModel->reload();
    QString themeName;
    if (m_isNetbook) {
        KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
        themeName = cg.readEntry("name", "air-netbook");
    } else {
        themeName = Plasma::Theme::defaultTheme()->themeName();
    }
    m_theme->setCurrentIndex(m_themeModel->indexOf(themeName));
    QApplication::restoreOverrideCursor();
}

void KCMDesktopTheme::detailChanged()
{
    m_bDetailsDirty = true;
    emit changed(true);
}

#include "kcmdesktoptheme.moc"

// vim: set noet ts=4:
