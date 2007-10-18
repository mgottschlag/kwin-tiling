/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "systemtraywidget.h"

// KDE
#include <KDebug>
#include <KWindowSystem>

// Qt
#include <QEvent>
#include <QHBoxLayout>
#include <QX11EmbedContainer>
#include <QX11Info>

// Xlib
#include <X11/Xlib.h>

namespace
{
enum
{
    SYSTEM_TRAY_REQUEST_DOCK,
    SYSTEM_TRAY_BEGIN_MESSAGE,
    SYSTEM_TRAY_CANCEL_MESSAGE
};
}

SystemTrayWidget::SystemTrayWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_layout = new QHBoxLayout(this);
    setLayout(m_layout);
    QPalette newPalette = palette();
    newPalette.setBrush(QPalette::Window, Qt::black);
    setPalette(newPalette);
    KWindowSystem::setState(winId(), NET::Sticky | NET::KeepAbove);
    init();
}

bool SystemTrayWidget::x11Event(XEvent *event)
{
    if (event->type == ClientMessage) {
        if (event->xclient.message_type == m_opcodeAtom &&
            event->xclient.data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
            embedWindow((WId)event->xclient.data.l[2]);
            return true;
        }
    }
    return QWidget::x11Event(event);
}

void SystemTrayWidget::init()
{
    Display *display = QX11Info::display();

    m_selectionAtom = XInternAtom(display, "_NET_SYSTEM_TRAY_S" + QByteArray::number(QX11Info::appScreen()), false);
    m_opcodeAtom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", false);
    XSetSelectionOwner(display, m_selectionAtom, winId(), CurrentTime);

    if (XGetSelectionOwner(display, m_selectionAtom) == winId()) {
        WId root = QX11Info::appRootWindow();
        XClientMessageEvent xev;

        xev.type = ClientMessage;
        xev.window = root;
        xev.message_type = XInternAtom(display, "MANAGER", false);
        xev.format = 32;
        xev.data.l[0] = CurrentTime;
        xev.data.l[1] = m_selectionAtom;
        xev.data.l[2] = winId();
        xev.data.l[3] = 0;  // manager specific data
        xev.data.l[4] = 0;  // manager specific data

        XSendEvent(display, root, false, StructureNotifyMask, (XEvent*)&xev);
    }
}

bool SystemTrayWidget::event(QEvent *event)
{
    if (event->type() == QEvent::LayoutRequest) {
        resize(minimumSize());
        emit sizeChanged();
    }
    return QWidget::event(event);
}

void SystemTrayWidget::embedWindow(WId id)
{
    kDebug() << "trying to add window with id " << id;
    if (! m_containers.contains(id)) {
        QX11EmbedContainer *container = new QX11EmbedContainer(this);
        container->embedClient(id);
        // TODO: add error handling
        m_layout->addWidget(container);
        container->show();
        m_containers[id] = container;
        connect(container, SIGNAL(clientClosed()), this, SLOT(windowClosed()) );
        kDebug() << "SystemTray: Window with id " << id << "added" << container;
    }
}

//what exactly is this for? is it related to QX11EmbedContainer::discardClient? why is it blank?
void SystemTrayWidget::discardWindow(WId)
{
}

void SystemTrayWidget::windowClosed()
{
    kDebug() << "Window closed";
    //by this point the window id is gone, so we have to iterate to find out who's lost theirs
    ContainersList::iterator i = m_containers.begin();
    while (i != m_containers.end()) {
        QX11EmbedContainer *c=i.value();
        if (c->clientWinId()==0) {
            i=m_containers.erase(i);
            kDebug() << "deleting container" << c;
            delete c;
            //do NOT assume that there will never be more than one without an id
            continue;
        }
        ++i;
    }
}

#include "systemtraywidget.moc"
