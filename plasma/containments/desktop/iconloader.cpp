/*
*   Copyright 2007 by Christopher Blauvelt <cblauvelt@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "iconloader.h"
#include "desktop.h"

#include <KUrl>
#include <KGlobalSettings>
#include <KDebug>

IconLoader::IconLoader(QObject *parent)
    : QObject(parent),
      m_desktop(0),
      m_showDeviceIcons(false)
{
}

IconLoader::~IconLoader()
{
}

void IconLoader::init(DefaultDesktop *desktop)
{
    if (!desktop) {
        return;
    }
    //multiple initiation guard
    if (desktop == m_desktop) {
        return;
    }
    m_desktop = desktop;
    m_iconMap.clear();

    connect(m_desktop, SIGNAL(appletRemoved(Plasma::Applet*)), this, SLOT(appletDeleted(Plasma::Applet*)));
    //build list of current icons
    foreach (Plasma::Applet* applet, m_desktop->applets()) {
        if (applet->name() == i18n("Icon")) {
            addIcon(applet);
        }
    }

    //list ~/Desktop and add new applets
    m_desktopDir.setAutoUpdate(true);
    m_desktopDir.setAutoErrorHandlingEnabled(false, 0);

    //connect(&m_desktopDir, SIGNAL(clear()), this, SLOT(clear()));
    connect(&m_desktopDir, SIGNAL(newItems(KFileItemList)), this, SLOT(newItems(KFileItemList)) );
    connect(&m_desktopDir, SIGNAL(deleteItem(KFileItem)), this, SLOT(deleteItem(KFileItem)));

    KUrl homedir(KGlobalSettings::desktopPath());
    m_desktopDir.openUrl(homedir);
}

void IconLoader::addIcon(KUrl url)
{
    QVariantList args;
    args << url.path();
    Plasma::Applet *newApplet = m_desktop->addApplet(QString("icon"),args,0);
    if (newApplet) {
        m_iconMap[url.path()] =  newApplet;
    }
}

void IconLoader::addIcon(Plasma::Applet *applet)
{
    KConfigGroup cg = applet->config();
    KUrl url = cg.readEntry(i18n("Url"), KUrl());
    if (url != KUrl()) {
        m_iconMap[url.path()] = applet;
    }
}

void IconLoader::deleteIcon(KUrl url)
{
    Plasma::Applet *applet = m_iconMap.value(url.path());
    if (applet) {
        m_iconMap.remove(url.path());
        applet->destroy();
    }
}

void IconLoader::deleteIcon(Plasma::Applet *applet)
{
    //we must be careful because this will be entered when the applet is already
    //in the qobject dtor so we can't invoke any applet methods
    m_iconMap.remove(m_iconMap.key(applet));
}

void IconLoader::newItems(const KFileItemList& items)
{
    if (!m_desktop) {
        return;
    }


    foreach (KFileItem item, items) {
        if (!m_iconMap.contains(item.url().path())) {
            addIcon(item.url());
        }
    }
}

void IconLoader::deleteItem(const KFileItem item)
{
    QString path = item.url().path();
    if (!m_iconMap.contains(path)) {
        kDebug() << "Icon " << path << " not found." << endl;
        return;
    }
    deleteIcon(item.url());
}

void IconLoader::appletDeleted(Plasma::Applet *applet)
{
    deleteIcon(applet);
}
