/***************************************************************************
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                        *
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

#ifndef SELECTIONBAR_H
#define SELECTIONBAR_H

#include <Plasma/ItemBackground>

class QTimer;

namespace Plasma
{
    class FrameSvg;
} // namespace Plasma

class ResultItem;

class SelectionBar : public Plasma::ItemBackground
{
    Q_OBJECT

public:
    SelectionBar(QGraphicsWidget *parent);

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

private Q_SLOTS:
    void acquireTarget();
    void actuallyHide();

private:
    QTimer *m_hideTimer;
};

#endif
