/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
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

#ifndef DASHBOARDVIEW_H
#define DASHBOARDVIEW_H

#include <QWeakPointer>

#include <Plasma/Plasma>
#include <Plasma/View>
#include <Plasma/WindowEffects>

class QAction;
class QToolButton;

namespace Plasma
{
    class WidgetExplorer;
    class Containment;
}

class DashboardView : public Plasma::View
{
    Q_OBJECT

public:
    DashboardView(Plasma::Containment* containment, Plasma::View *parent);
    ~DashboardView();
    bool eventFilter(QObject *watched, QEvent *event);

protected:
    void drawBackground(QPainter * painter, const QRectF & rect);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void toggleVisibility();
    void showDashboard(bool showDashboard);

    /**
     * Sets the containment for this view, which will also cause the view
     * to track the geometry of the containment.
     *
     * @arg containment the containment to center the view on
     */
    void setContainment(Plasma::Containment *newContainment);

Q_SIGNALS:
    void dashboardClosed();

private Q_SLOTS:
    void showWidgetExplorer();
    void hideView();
    void suppressShowTimeout();
    void compositingChanged(bool);

private:
    Plasma::View *m_view;
    QWeakPointer<Plasma::WidgetExplorer> m_widgetExplorer;
    QToolButton *m_closeButton;
    QAction *m_hideAction;
    bool m_suppressShow : 1;
    bool m_zoomIn : 1;
    bool m_zoomOut : 1;
    bool m_init : 1;
};

#endif // multiple inclusion guard
