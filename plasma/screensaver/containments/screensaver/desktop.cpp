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

//#define DEBUG_CONSOLE
#ifdef DEBUG_CONSOLE
#include <QGraphicsLinearLayout>
#include <Plasma/Frame>
#include <Plasma/TextBrowser>
#endif

#include <../../../desktop/toolboxes/internaltoolbox.h>

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

    // trigger the creation of the toolbox. ugly! :) but nicer than having our own toolbox plugin
    // just for the screensaver?
    QAction *dummy = new QAction(this);
    addToolBoxAction(dummy);
    removeToolBoxAction(dummy);
    delete dummy;

    Plasma::AbstractToolBox *tools = toolBox();
    InternalToolBox *internal = dynamic_cast<InternalToolBox *>(tools);

#ifdef DEBUG_CONSOLE
    Plasma::Frame *f = new Plasma::Frame(this);
    f->setGeometry(QRectF(100, 100, 500, 500));
    QGraphicsLinearLayout *l = new QGraphicsLinearLayout(f);
    l->addItem(msgs);
    Plasma::TextBrowser *msgs new Plasma::TextBrowser(this);
    msgs->setText(QString("got %1 with %2 actions").arg((int)internal).arg(tools ?
                tools->metaObject()->className() : "NADA"));
#endif

    if (internal) {
        // remove all the actions pre-made for us here
        foreach (QAction *action, internal->actions()) {
            internal->removeTool(action);
        }
    }

    // add our own actions. huzzah.
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

void SaverDesktop::newApplet(Plasma::Applet *applet, const QPointF &pos)
{
    Q_UNUSED(pos);
    applet->installSceneEventFilter(this);
}

K_EXPORT_PLASMA_APPLET(saverdesktop, SaverDesktop)

#include "desktop.moc"
