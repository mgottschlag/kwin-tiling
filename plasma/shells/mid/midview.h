/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef MIDVIEW_H
#define MIDVIEW_H

#include "plasma/plasma.h"
#include "plasma/view.h"

namespace Plasma
{
    class Containment;
} // namespace Plasma

class DashboardView;
class BackgroundDialog;

class MidView : public Plasma::View
{
    Q_OBJECT

public:
    MidView(Plasma::Containment *containment, QWidget *parent = 0);
    ~MidView();

    /**
     * hook up all needed signals to a containment
     */
    void connectContainment(Plasma::Containment *containment);

    /**
     * Sets this MidView as a desktop window if @p isDesktop is
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

    static int defaultId() { return 1; }

public slots:
    void showAppletBrowser();

    /**
     * reimplemented from Plasma::View
     */
    void setContainment(Plasma::Containment *containment);

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
     * Configure containment.
     *
     * @arg containment to configure
     */
    void configureContainment();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
};

#endif // multiple inclusion guard
