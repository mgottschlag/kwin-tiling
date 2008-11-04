/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <Plasma/Plasma>
#include <Plasma/View>

namespace Plasma
{
    class Containment;
} // namespace Plasma

class DashboardView;

class DesktopView : public Plasma::View
{
    Q_OBJECT

public:
    DesktopView(Plasma::Containment *containment, int id, QWidget *parent);
    ~DesktopView();

    /**
     * Sets this DesktopView as a desktop window if @p isDesktop is
     * true or an ordinary window otherwise.
     *
     * Desktop windows are displayed beneath all other windows, have
     * no window decoration and occupy the full size of the desktop.
     */
    void setIsDesktop(bool isDesktop);

    /**
     * Returns true if this widget is currently a desktop window.
     * See setAsDesktop()
     */
    bool isDesktop() const;

    /**
     * Returns whether or not the dashboard view is visible
     */
    bool isDashboardVisible() const;

public slots:
    /**
     * zoom in towards the given containment.
     * if toContainment is null, the current containment is used instead.
     * zooming in also sets toContainment as current.
     */
    void zoomIn(Plasma::ZoomLevel zoomLevel);
    void zoomOut(Plasma::ZoomLevel zoomLevel);
    void toggleDashboard();
    void adjustSize();

    void screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment);

    /**
     * switch to the "next" available containment on the corona.
     */
    void nextContainment();

    /**
     * switch to the "previous" available containment on the corona.
     */
    void previousContainment();

    /**
     * Sets the containment for this view, which will also cause the view
     * to track the geometry of the containment.
     *
     * @arg containment the containment to center the view on
     */
    void setContainment(Plasma::Containment *containment);

protected:
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    DashboardView *m_dashboard;
    bool m_dashboardFollowsDesktop;
};

#endif // multiple inclusion guard
