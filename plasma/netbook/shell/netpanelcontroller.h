/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
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

#ifndef NETPANELCONTROLLER_H
#define NETPANELCONTROLLER_H

#include <Plasma/Dialog>

namespace Plasma
{
    class Containment;
    class ToolButton;
    class Svg;
    class View;
}

class QGraphicsWidget;
class QGraphicsLinearLayout;

class NetView;

class NetPanelController : public Plasma::Dialog
{
    Q_OBJECT
public:
    NetPanelController(QWidget *parent = 0, NetView *view = 0, Plasma::Containment *containment = 0);
    ~NetPanelController();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void updateFormFactor();
    void resizeEvent(QResizeEvent *e);

private Q_SLOTS:
    void updatePosition();

private:
    Plasma::Containment *m_containment;
    Plasma::View *m_view;
    QGraphicsWidget *m_mainWidget;
    QGraphicsLinearLayout *m_layout;
    Plasma::ToolButton *m_moveButton;
    Plasma::ToolButton *m_resizeButton;
    Plasma::ToolButton *m_autoHideButton;
    Plasma::ToolButton *m_watched;
    Plasma::Svg *m_iconSvg;
};

#endif
