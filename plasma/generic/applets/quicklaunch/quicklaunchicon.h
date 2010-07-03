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
#ifndef QUICKLAUNCH_QUICKLAUNCHICON_H
#define QUICKLAUNCH_QUICKLAUNCHICON_H

// KDE
#include <KUrl>

// Plasma
#include <Plasma/IconWidget>

// Own
#include "itemdata.h"

class QGraphicsItem;

namespace Quicklaunch {

class QuicklaunchIcon : public Plasma::IconWidget
{
    Q_OBJECT

public:
    QuicklaunchIcon(const ItemData &data, QGraphicsItem *parent = 0);

    ~QuicklaunchIcon();

    void setIconNameVisible(bool enable);
    bool isIconNameVisible();

    void setItemData(const ItemData &data);

    ItemData itemData() const;
    KUrl url() const;


public Q_SLOTS:
    void execute();
    void toolTipAboutToShow();
    void toolTipHidden();

private:
    ItemData m_itemData;
    bool m_iconNameVisible;
};
}

#endif /* QUICKLAUNCH_QUICKLAUNCHICON_H */
