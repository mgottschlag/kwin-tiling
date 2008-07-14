/*
 *   Copyright 2006, 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_APP_H
#define PLASMA_APP_H

#include <QList>

#include <KUniqueApplication>

namespace Plasma
{
    class Containment;
    class Corona;
} // namespace Plasma

class SaverView;

class PlasmaApp : public KUniqueApplication
{
    Q_OBJECT
public:
    ~PlasmaApp();

    static PlasmaApp* self();
    static bool hasComposite();

    Plasma::Corona* corona();

    /**
     * enables or disables cheats useful for debugging.
     * FIXME maybe this shouldn't be public.
     */
    void enableCheats(bool enable);

    /** 
     * Returns true if cheats are enabled
     * @see enableCheats
     */
    bool cheatsEnabled() const;

public Q_SLOTS:
    // DBUS interface. if you change these methods, you MUST run:
    // qdbuscpp2xml plasmaapp.h -o org.kde.plasma.App.xml
    void showPlasma();
    void hidePlasma();

private Q_SLOTS:
    void cleanup();
    void createView(Plasma::Containment *containment);
    void adjustSize(int screen);

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);

    Plasma::Corona *m_corona;
    SaverView *m_view;
    bool m_cheats;
};

#endif // multiple inclusion guard
