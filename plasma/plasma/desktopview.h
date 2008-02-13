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

#include "plasma/plasma.h"
#include "plasma/view.h"

class DashboardView;

class DesktopView : public Plasma::View
{
    Q_OBJECT

public:
    DesktopView(int screen, QWidget *parent);
    ~DesktopView();

public slots:
    void zoomIn();
    void zoomOut();
    void showAppletBrowser();
    void toggleDashboard();
    void adjustSize();

protected:
    void wheelEvent(QWheelEvent *event);

private:
    Plasma::ZoomLevel m_zoomLevel;
    DashboardView *m_dashboard;
};

#endif // multiple inclusion guard
