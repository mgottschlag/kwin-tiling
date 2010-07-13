/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>            *
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
#include "quicklaunchicon.h"

// Qt
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>

// KDE
#include <KRun>

// Plasma
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

// Own
#include "itemdata.h"

using Plasma::ToolTipManager;
using Plasma::ToolTipContent;

namespace Quicklaunch {

QuicklaunchIcon::QuicklaunchIcon(const ItemData &data, QGraphicsItem *parent)
  : Plasma::IconWidget(parent),
    m_itemData(data),
    m_iconNameVisible(false)
{
    setIcon(data.icon());

    ToolTipManager::self()->registerWidget(this);
    connect(this, SIGNAL(clicked()), SLOT(execute()));
    setOwnedByLayout(true);
}

void QuicklaunchIcon::setIconNameVisible(bool enable)
{
    if (enable == m_iconNameVisible) {
        return;
    }

    m_iconNameVisible = enable;

    if (enable) {
        setText(m_itemData.name());
    } else {
        setText(QString::null);
    }
}

bool QuicklaunchIcon::isIconNameVisible()
{
    return m_iconNameVisible;
}

void QuicklaunchIcon::setItemData(const ItemData &data)
{
    setIcon(data.icon());
    if (m_iconNameVisible) {
        setText(data.name());
    }

    if (ToolTipManager::self()->isVisible(this)) {
        updateToolTipContent();
    }

    m_itemData = data;
}

ItemData QuicklaunchIcon::itemData() const
{
    return m_itemData;
}

KUrl QuicklaunchIcon::url() const
{
    return m_itemData.url();
}

void QuicklaunchIcon::execute()
{
    new KRun(m_itemData.url(), 0);
}

void QuicklaunchIcon::toolTipAboutToShow()
{
    updateToolTipContent();
}

void QuicklaunchIcon::toolTipHidden()
{
    ToolTipManager::self()->clearContent(this);
}

void QuicklaunchIcon::updateToolTipContent()
{
    ToolTipContent toolTipContent;
    toolTipContent.setMainText(m_itemData.name());
    toolTipContent.setSubText(m_itemData.description());
    toolTipContent.setImage(icon());

    ToolTipManager::self()->setContent(this, toolTipContent);
}

}
