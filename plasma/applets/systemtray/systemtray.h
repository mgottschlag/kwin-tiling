/***************************************************************************
 *   systemtray.h                                                          *
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

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

// Own
#include "systemtraywidget.h"

// Qt
#include <QPointer>

// Plasma
#include <plasma/applet.h>

namespace Plasma
{
    class PanelSvg;
}

class SystemTray: public Plasma::Applet
{
Q_OBJECT

public:
    explicit SystemTray(QObject *parent, const QVariantList &arguments = QVariantList());
    ~SystemTray();
  
    void constraintsEvent(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);

private slots:
    void updateWidgetGeometry();

private:
    void updateWidgetOrientation();

    Plasma::PanelSvg *m_background;
    bool m_showOwnBackground;

    // The parent widget might delete this so we guard it
    QPointer<SystemTrayWidget> m_systemTrayWidget;
};

K_EXPORT_PLASMA_APPLET(systemtray, SystemTray)

#endif // SYSTEMTRAY_H
