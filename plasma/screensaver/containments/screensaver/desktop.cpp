/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Chani Armitage <chanika@gmail.com>
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

#include "desktop.h"

#include <QAction>

#include <KDebug>

#include <Plasma/Corona>
#include <Plasma/Theme>

using namespace Plasma;

SaverDesktop::SaverDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_lockDesktopAction(0),
      m_appletBrowserAction(0)
{
        setContainmentType(CustomContainment);
        connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), SLOT(newApplet(Plasma::Applet*,QPointF)));
        setHasConfigurationInterface(true);
}

SaverDesktop::~SaverDesktop()
{
}

void SaverDesktop::init()
{
    Containment::init();

    //remove the desktop actions
    QAction *unwanted = action("zoom in");
    delete unwanted;
    unwanted = action("zoom out");
    delete unwanted;
    unwanted = action("add sibling containment");
    delete unwanted;

    QAction *leave = corona()->action("unlock desktop");
    if (leave) {
        addToolBoxAction(leave);
    }

    QAction *lock = corona()->action("unlock widgets");
    if (lock) {
        addToolBoxAction(lock);
    }

    QAction *a = action("configure");
    if (a) {
        a->setText(i18n("Settings"));
        addToolBoxAction(a);
    }

    a = action("add widgets");
    if (a) {
        addToolBoxAction(a);
    }
}

QList<QAction*> SaverDesktop::contextualActions()
{
    if (!m_appletBrowserAction) {
        m_appletBrowserAction = action("add widgets");
        m_lockDesktopAction = corona()->action("unlock widgets");
    }
    QAction *config = action("configure");
    QAction *quit = corona()->action("unlock desktop");

    QList<QAction*> actions;
    actions.append(m_appletBrowserAction);
    if (config) {
        actions.append(config);
    }
    actions.append(m_lockDesktopAction);
    actions.append(quit);

    return actions;
}

void SaverDesktop::newApplet(Plasma::Applet *applet, const QPointF &pos)
{
    Q_UNUSED(pos);
    applet->installSceneEventFilter(this);
}

K_EXPORT_PLASMA_APPLET(saverdesktop, SaverDesktop)

#include "desktop.moc"
