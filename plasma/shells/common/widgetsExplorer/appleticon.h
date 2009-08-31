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

#include <QGraphicsWidget>

#include <plasma/framesvg.h>

class AppletIconWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        virtual ~AppletIconWidget();

        void setIconHeight(int height);
        void setAppletItem(PlasmaAppletItem *appletIcon);
        void setSelected(bool selected);
        PlasmaAppletItem *appletItem();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);
        void selected(AppletIconWidget *applet);
        void doubleClicked(AppletIconWidget *applet);

    protected:
        //listen to events and emit signals
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void resizeEvent(QGraphicsSceneResizeEvent *);

    private:
        PlasmaAppletItem *m_appletItem;
        bool m_selected;
        bool m_hovered;
        int m_iconHeight;
        Plasma::FrameSvg *m_selectedBackgroundSvg;
};

#endif //APPLETICON_H
