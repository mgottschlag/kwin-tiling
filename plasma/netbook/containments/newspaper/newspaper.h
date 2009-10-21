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
class AppletTitleBar;
class NetToolBox;

namespace Plasma
{
    class FrameSvg;
    class ScrollWidget;
    class SvgWidget;
    class ToolButton;
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

protected:
    void createAppletTitle(Plasma::Applet *applet);
    void changeEvent(QEvent *event);

    void saveContents(KConfigGroup &group) const;
    void restore(KConfigGroup &group);

    QGraphicsLinearLayout *addColumn();
    void removeColumn(int column);

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

private Q_SLOTS:
    void toggleImmutability();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void cleanupColumns();
    void updateSize();
    void updateConfigurationMode(bool config);
    void addNewsPaper();

    void goLeft();
    void goRight();
    void scrollTimeout();

private:
    QGraphicsWidget *m_mainWidget;
    Plasma::ScrollWidget *m_scrollWidget;
    QGraphicsLinearLayout *m_externalLayout;
    QGraphicsLinearLayout *m_mainLayout;
    Qt::Orientation m_orientation;
    Plasma::FrameSvg *m_background;
    AppletOverlay *m_appletOverlay;
    NetToolBox *m_toolBox;
    bool m_dragging;
    Plasma::ToolButton *m_leftArrow;
    Plasma::ToolButton *m_rightArrow;
    QTimer *m_scrollTimer;
};


#endif // PLASMA_PANEL_H
