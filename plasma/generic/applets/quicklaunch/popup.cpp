/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>     *
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "popup.h"

// Qt
#include <Qt>
#include <QtCore/QMargins>
#include <QtCore/QSize>

// KDE
#include <KIconLoader>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Dialog>

// Own
#include "iconarea.h"
#include "icongridlayout.h"
#include "quicklaunch.h"

using Plasma::Applet;
using Plasma::Corona;
using Plasma::Dialog;

namespace Quicklaunch {

Popup::Popup(Quicklaunch *applet)
:
    Dialog(0, Qt::X11BypassWindowManagerHint),
    m_applet(applet),
    m_iconArea(new IconArea())
{
    m_applet->containment()->corona()->addItem(m_iconArea);
    m_applet->containment()->corona()->addOffscreenWidget(m_iconArea);

    m_iconArea->setIconNamesVisible(false);

    m_iconArea->layout()->setCellSizeHint(KIconLoader::SizeMedium);

    setGraphicsWidget(m_iconArea);

    connect(m_applet, SIGNAL(geometryChanged()), SLOT(onAppletGeometryChanged()));
    connect(m_iconArea, SIGNAL(iconClicked()), SLOT(onIconClicked()));
    connect(m_iconArea, SIGNAL(displayedItemCountChanged()), SLOT(onDisplayedItemCountChanged()));
}

Popup::~Popup()
{
    Dialog::close();
    delete m_iconArea;
}

IconArea * Popup::iconArea()
{
    return m_iconArea;
}

void Popup::show()
{
    Dialog::show();
    syncSizeAndPosition();
}

void Popup::onAppletGeometryChanged()
{
    syncSizeAndPosition();
}

void Popup::onDisplayedItemCountChanged()
{
    syncSizeAndPosition();
}

void Popup::onIconClicked()
{
    hide();
}

void Popup::syncSizeAndPosition()
{
    if (!isVisible()) {
        return;
    }

    int displayedItemCount = m_iconArea->layout()->count();

    const int iconAreaWidth = displayedItemCount *
        (KIconLoader::SizeMedium + m_iconArea->layout()->cellSpacing()) -
        m_iconArea->layout()->cellSpacing();

    const int iconAreaHeight = KIconLoader::SizeMedium;

    QMargins margins = contentsMargins();

    QSize newSize(
        iconAreaWidth + margins.left() + margins.right(),
        iconAreaHeight + margins.top() + margins.bottom());

    move(m_applet->popupPosition(newSize, Qt::AlignRight));
    resize(newSize);
}
}

#include "popup.moc"
