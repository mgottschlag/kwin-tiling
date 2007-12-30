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

#ifndef PLASMA_ICONLOADER_H
#define PLASMA_ICONLOADER_H

//#include "desktop.h"

#include <QObject>
#include <QMap>
#include <QString>

#include <KDirLister>
#include <KFileItem>
#include <plasma/applet.h>

class DefaultDesktop;

class IconLoader : public QObject
{
    Q_OBJECT

public:
    IconLoader(QObject *parent=0);
    ~IconLoader();
    void init(DefaultDesktop *desktop);

    bool showDeviceIcons();
    void setShowDeviceIcons(bool show);


private:
    void addIcon(const KUrl &url);
    void addIcon(Plasma::Applet *applet);
    void deleteIcon(const KUrl &url);
    void deleteIcon(Plasma::Applet *applet);
    
    KDirLister m_desktopDir;
    QMap<QString, Plasma::Applet*> m_iconMap;
    DefaultDesktop *m_desktop;
    bool m_showDeviceIcons;

private Q_SLOTS:
    //void clear();
    void newItems(const KFileItemList& items);
    void deleteItem(const KFileItem item);
    void appletDeleted(Plasma::Applet *applet);
};

#endif
