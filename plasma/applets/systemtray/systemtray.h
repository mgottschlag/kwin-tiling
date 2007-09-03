/***************************************************************************
 *   systemtray.h                                                          *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
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

// Plasma
#include <Plasma/Applet>

class SystemTrayWidget;

class SystemTray: public Plasma::Applet
{
Q_OBJECT

public:
    SystemTray(QObject *parent, const QVariantList &arguments = QVariantList());
    ~SystemTray();

    QSizeF contentSizeHint() const;

    Qt::Orientations expandingDirections() const;

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

private slots:
    void updateLayout()
    { update(); }

private:
    SystemTrayWidget *m_systemTrayWidget;
};

K_EXPORT_PLASMA_APPLET(systemtray, SystemTray)

#endif // SYSTEMTRAY_H
