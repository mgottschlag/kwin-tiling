/***************************************************************************
 *   applet.h                                                              *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef APPLET_H
#define APPLET_H

#include <plasma/popupapplet.h>

namespace SystemTray
{
    class Notification;
}


namespace SystemTray
{

class Applet : public Plasma::PopupApplet
{
    Q_OBJECT

public:
    explicit Applet(QObject *parent, const QVariantList &arguments = QVariantList());

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    void setGeometry(const QRectF &rect);

protected:
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    void createConfigurationInterface(KConfigDialog *parent);
    void initExtenderItem(Plasma::ExtenderItem *extenderItem);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }

private slots:
    void configAccepted();
    void propogateSizeHintChange(Qt::SizeHint which);
    void checkSizes();
    void addNotification(SystemTray::Notification *notification);
    void hidePopupIfEmpty();

private:
    class Private;
    Private* const d;
};

}


#endif
