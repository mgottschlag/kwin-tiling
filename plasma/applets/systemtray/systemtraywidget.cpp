/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
 *   Copyright (C) 2007 Jason Stubbs <jasonbstubbs@gmail.com>              *
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
#include "systemtraycontainer.h"

// Qt
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

SystemTrayWidget::SystemTrayWidget(QWidget *parent)
    : QWidget(parent),
    m_orientation(Qt::Horizontal),
    m_nextRow(0),
    m_nextColumn(0)
{
    m_mainLayout = new QGridLayout(this);

    // Override spacing set by the current style
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(4);

    init();
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

bool SystemTrayWidget::x11Event(XEvent *event)
{
    if (event->type == ClientMessage) {
        if (event->xclient.message_type == m_opcodeAtom &&
            event->xclient.data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {

            // Set up a SystemTrayContainer for the client
            SystemTrayContainer *container = new SystemTrayContainer(this);
            addWidgetToLayout(container);

            connect(container, SIGNAL(clientIsEmbedded()), this, SIGNAL(sizeShouldChange()));
            connect(container, SIGNAL(destroyed(QObject *)), this, SLOT(relayoutContainers(QObject *)));

            const WId systemTrayClientId = (WId)event->xclient.data.l[2];

            container->embedSystemTrayClient(systemTrayClientId);

            return true;
        }
    }
    return QWidget::x11Event(event);
}

void SystemTrayWidget::setOrientation(Qt::Orientation orientation)
{
    if (orientation != m_orientation) {
        m_orientation = orientation;
        relayoutContainers();
    }
}

void SystemTrayWidget::addWidgetToLayout(QWidget *widget)
{
    // Figure out where it should go and add it to our layout

    if (m_orientation == Qt::Horizontal) {
        // Add down then across when horizontal
        if (m_nextRow == m_mainLayout->rowCount()
            && m_mainLayout->minimumSize().height() + m_mainLayout->spacing()
               + widget->minimumHeight() > maximumHeight() - 2 * MARGIN) {
            m_nextColumn++;
            m_nextRow = 0;
        }
        m_mainLayout->addWidget(widget, m_nextRow, m_nextColumn);
        m_nextRow++;
    } else {
        // Add across then down when vertical
        if (m_nextColumn == m_mainLayout->columnCount()
            && m_mainLayout->minimumSize().width() + m_mainLayout->spacing()
               + widget->minimumWidth() > maximumWidth() - 2 * MARGIN) {
            m_nextRow++;
            m_nextColumn = 0;
        }
        m_mainLayout->addWidget(widget, m_nextRow, m_nextColumn);
        m_nextColumn++;
    }
}

void SystemTrayWidget::relayoutContainers(QObject *removeContainer)
{
    // Pull all widgets from our container, skipping over the one that was just
    // deleted
    QList<QWidget *> remainingWidgets;
    while (QLayoutItem* item = m_mainLayout->takeAt(0)) {
        if (item->widget() && item->widget() != removeContainer) {
            remainingWidgets.append(item->widget());
        }
        delete item;
    }

    // Reset the widths and heights in our layout to 0 so that the removed
    // widget's space isn't kept
    // (Why doesn't QGridLayout do this automatically?)
    for (int row = 0; row < m_mainLayout->rowCount(); row++) {
        m_mainLayout->setRowMinimumHeight(row, 0);
    }
    for (int column = 0; column < m_mainLayout->columnCount(); column++) {
        m_mainLayout->setColumnMinimumWidth(column, 0);
    }

    // Re-add remaining widgets
    m_nextRow = 0;
    m_nextColumn = 0;
    foreach (QWidget *widget, remainingWidgets) {
        addWidgetToLayout(widget);
    }

    // Force a layout so that minimumSizeHint() returns the correct value and
    // signal that our size should change
    layout()->activate();
    emit sizeShouldChange();
}

#include "systemtraywidget.moc"
