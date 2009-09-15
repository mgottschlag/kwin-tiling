/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef CONTROLLERWINDOW_H
#define CONTROLLERWINDOW_H

#include <QWidget>

#include <Plasma/Plasma>

class QBoxLayout;
class QGraphicsView;

namespace Plasma
{
    class Containment;
    class FrameSvg;
    class WidgetExplorer;
} // namespace Plasma

class ControllerWindow : public QWidget
{
    Q_OBJECT

public:
    ControllerWindow(QWidget* parent);
    ~ControllerWindow();

    void setContainment(Plasma::Containment *containment);
    Plasma::Containment *containment() const;

    QSize sizeHint() const;

    void setLocation(const Plasma::Location &loc);
    Plasma::Location location() const;
    Qt::Orientation orientation() const;

    void showWidgetsExplorer();
    bool widgetsExplorerIsVisible() const;

    bool eventFilter(QObject *watched, QEvent *event);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
    void onActiveWindowChanged(WId id);

private:
    Plasma::Location m_location;
    QBoxLayout *m_layout;
    Plasma::FrameSvg *m_background;
    Plasma::Containment *m_containment;
    QGraphicsView *m_widgetExplorerView;
    Plasma::WidgetExplorer *m_widgetExplorer;
};

#endif

