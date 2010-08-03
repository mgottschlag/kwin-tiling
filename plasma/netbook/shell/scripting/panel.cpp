/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "panel.h"


#include <Plasma/Corona>
#include <Plasma/Containment>

#include "netview.h"
#include "plasmaapp.h"
#include <plasmagenericshell/scripting/scriptengine.h>
#include <plasmagenericshell/scripting/widget.h>

namespace WorkspaceScripting
{

NetPanel::NetPanel(Plasma::Containment *containment, QObject *parent)
    : Containment(containment, parent)
{
}

NetPanel::~NetPanel()
{
}

QString NetPanel::location() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return "floating";
    }

    switch (c->location()) {
        case Plasma::Floating:
            return "floating";
            break;
        case Plasma::Desktop:
            return "desktop";
            break;
        case Plasma::FullScreen:
            return "fullscreen";
            break;
        case Plasma::TopEdge:
            return "top";
            break;
        case Plasma::BottomEdge:
            return "bottom";
            break;
        case Plasma::LeftEdge:
            return "left";
            break;
        case Plasma::RightEdge:
            return "right";
            break;
    }

    return "floating";
}

void NetPanel::setLocation(const QString &locationString)
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    const QString lower = locationString.toLower();
    Plasma::Location loc = Plasma::Floating;
    if (lower == "desktop") {
        loc = Plasma::Desktop;
    } else if (lower == "fullscreen") {
        loc = Plasma::FullScreen;
    } else if (lower == "top") {
        loc = Plasma::TopEdge;
    } else if (lower == "bottom") {
        loc = Plasma::BottomEdge;
    } else if (lower == "left") {
        loc = Plasma::LeftEdge;
    } else if (lower == "right") {
        loc = Plasma::RightEdge;
    }

    c->setLocation(loc);
}

NetView *NetPanel::panel() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    return PlasmaApp::self()->controlBar();
}

int NetPanel::height() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    return c->formFactor() == Plasma::Vertical ? c->size().width()
                                               : c->size().height();
}

void NetPanel::setHeight(int height)
{
    Plasma::Containment *c = containment();
    if (height < 16 || !c) {
        return;
    }

    NetView *v = panel();
    if (v) {
        QRect screen = c->corona()->screenGeometry(v->screen());
        QSizeF size = c->size();
        const int max = (c->formFactor() == Plasma::Vertical ? screen.width() : screen.height()) / 3;
        height = qBound(16, height, max);

        if (c->formFactor() == Plasma::Vertical) {
            size.setWidth(height);
        } else {
            size.setHeight(height);
        }

        c->resize(size);
        c->setMinimumSize(size);
        c->setMaximumSize(size);
    }
}

bool NetPanel::autoHide() const
{
    NetView *v = panel();
    if (v) {
        return v->autoHide();
    }

    return false;
}

void NetPanel::setAutoHide(const bool autoHide)
{
    NetView *v = panel();
    if (v && autoHide != v->autoHide()) {
        v->setAutoHide(autoHide);
    }
}

}

#include "panel.moc"

