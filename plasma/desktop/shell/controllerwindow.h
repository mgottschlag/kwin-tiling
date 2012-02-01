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
#include <Plasma/Dialog>

class QBoxLayout;
class QGraphicsWidget;

namespace Plasma
{
    class Containment;
    class Corona;
    class FrameSvg;
    class WidgetExplorer;
    class Dialog;
} // namespace Plasma

namespace Kephal
{
    class Screen;
}

class ActivityManager;

class ControllerWindow : public QWidget
{
    Q_OBJECT

public:
    ControllerWindow(QWidget* parent);
    ~ControllerWindow();

    virtual void setContainment(Plasma::Containment *containment);
    Plasma::Containment *containment() const;

    virtual void setLocation(const Plasma::Location &loc);
    Plasma::Location location() const;
    Qt::Orientation orientation() const;
    QPoint positionForPanelGeometry(const QRect &panelGeom) const;

    bool isControllerViewVisible() const;

    Plasma::FrameSvg *background() const;

    int screen() const;
    void setScreen(int screen);

    bool showingWidgetExplorer() const;
    bool showingActivityManager() const;

public Q_SLOTS:
    void activate();
    void showWidgetExplorer();
    void showActivityManager();
    virtual void closeIfNotFocussed();

protected:
    void setGraphicsWidget(QGraphicsWidget *widget);

    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void backgroundChanged();
    void adjustAndSetMaxSize();
    void syncToGraphicsWidget();

private:
    Plasma::Location m_location;
    QBoxLayout *m_layout;
    Plasma::FrameSvg *m_background;
    QWeakPointer<Plasma::Containment> m_containment;
    int m_screen;
    QGraphicsView *m_view;
    QGraphicsWidget *m_watchedWidget;
    ActivityManager *m_activityManager;
    Plasma::WidgetExplorer *m_widgetExplorer;
    QGraphicsWidget *m_graphicsWidget;
    QTimer *m_adjustViewTimer;
    bool m_ignoredWindowClosed;
};

#endif

