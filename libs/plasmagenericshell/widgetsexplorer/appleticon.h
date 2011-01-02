/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLETICON_H
#define APPLETICON_H

#include "plasmaappletitemmodel_p.h"
#include "abstracticon.h"

#include <QWeakPointer>

class AppletIconWidget : public Plasma::AbstractIcon
{
    Q_OBJECT

    public:
        explicit AppletIconWidget(PlasmaAppletItem *appletItem);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        PlasmaAppletItem *appletItem();

        QPixmap pixmap(const QSize &size);
        QMimeData* mimeData();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    protected Q_SLOTS:
        void itemChanged(QStandardItem *item);

    private:
        QWeakPointer<PlasmaAppletItem> m_appletItem;
        KIcon m_runningIcon;
};

#endif //APPLETICON_H
