/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2009 by Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_NEWSPAPER_H
#define PLASMA_NEWSPAPER_H

#include <Plasma/Containment>

class QGraphicsLinearLayout;
class AppletOverlay;

namespace Plasma
{
    class FrameSvg;
    class ScrollWidget;
    class SvgWidget;
}

class Newspaper : public Plasma::Containment
{
    Q_OBJECT
    friend class AppletOverlay;

public:
    Newspaper(QObject *parent, const QVariantList &args);
    ~Newspaper();
    void init();

    void constraintsEvent(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);


private slots:

    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void themeUpdated();
    void updateSize();
    void updateConfigurationMode(bool config);
    void containmentAdded(Plasma::Containment *containment);

private:

    QGraphicsWidget *m_mainWidget;
    Plasma::ScrollWidget *m_scrollWidget;
    QGraphicsLinearLayout *m_externalLayout;
    QGraphicsLinearLayout *m_mainLayout;
    QGraphicsLinearLayout *m_leftLayout;
    QGraphicsLinearLayout *m_rightLayout;
    Qt::Orientation m_orientation;
    Plasma::FrameSvg *m_background;
    AppletOverlay *m_appletOverlay;
    bool m_dragging;
};


#endif // PLASMA_PANEL_H
