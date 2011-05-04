/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PLASMA_PANELSHADOWS_H
#define PLASMA_PANELSHADOWS_H

#include <QSet>

#include <Plasma/Svg>

class PanelShadows : Plasma::Svg
{
    Q_OBJECT

public:
    PanelShadows(QObject *parent = 0);

    void addWinId(WId id);
    void removeWinId(WId id);

private Q_SLOTS:
    void updateShadows();

private:
    void setupPixmaps();
    void updateShadows(WId wid);

    QList<QPixmap> m_shadowPixmaps;
    QSet<WId> m_wids;
};

#endif

