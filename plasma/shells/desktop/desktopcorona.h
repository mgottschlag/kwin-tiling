/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef DESKTOPCORONA_H
#define DESKTOPCORONA_H

#include <QtGui/QGraphicsScene>

#include <Plasma/Corona>

namespace Plasma
{
    class Applet;
} // namespace Plasma

namespace Kephal {
    class Screen;
} // namespace Kephal

/**
 * @short A Corona with desktop-y considerations
 */
class DesktopCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit DesktopCorona(QObject * parent = 0);

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout();

    /**
     * Ensures we have a desktop containment for every screen
     */
    void checkScreens();

    virtual int numScreens() const;
    virtual QRect screenGeometry(int id) const;
    virtual QRegion availableScreenRegion(int id) const;

protected Q_SLOTS:
    void screenAdded(Kephal::Screen *s);

private:
    void init();
    void addDesktopContainment(int screen, int desktop = -1);
    Plasma::Applet *loadDefaultApplet(const QString &pluginName, Plasma::Containment *c);
};

#endif


