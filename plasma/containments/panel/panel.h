/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
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

#ifndef PLASMA_PANEL_H
#define PLASMA_PANEL_H

#include <plasma/containment.h>

namespace Plasma
{
    class Svg;
}

class Panel : public Plasma::Containment
{
    Q_OBJECT
public:
    Panel(QObject *parent, const QVariantList &args);
    ~Panel();

    void constraintsUpdated(Plasma::Constraints constraints);
    Qt::Orientations expandingDirections() const;

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    void paintBackground(QPainter *painter, const QRect &contentsRect);
private:
    Plasma::Svg *m_background;
    QPixmap* m_cachedBackground;
    bool m_drawTop : 1;
    bool m_drawLeft : 1;
    bool m_drawRight : 1;
    bool m_drawBottom : 1;
};


#endif // PLASMA_PANEL_H
