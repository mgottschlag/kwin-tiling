/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
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
#include "systemtraycontainer.h"

// KDE
#include <KDebug>

// Qt
#include <QX11Info>

// Xlib
#include <X11/Xlib.h>

SystemTrayContainer::SystemTrayContainer(WId clientId, QWidget *parent)
    : KX11EmbedContainer(clientId, parent)
{
    connect(this, SIGNAL(clientClosed()), SLOT(deleteLater()));
    connect(this, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(handleError(QX11EmbedContainer::Error)));

    // Tray icons have a fixed size of 22x22
    setMinimumSize(22, 22);

    kDebug() << "attempting to embed" << clientId;
    embedClient(clientId);

#if 0
    // BUG: error() sometimes return Unknown even on success
    if (error() == Unknown || error() == InvalidWindowID) {
        kDebug() << "embedding failed for" << clientId;
        deleteLater();
    }
#endif
}

void SystemTrayContainer::handleError(QX11EmbedContainer::Error error)
{
    Q_UNUSED(error);
    deleteLater();
}
