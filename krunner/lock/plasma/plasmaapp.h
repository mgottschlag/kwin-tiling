/*
 *   Copyright 2006, 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
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

Q_SIGNALS:
    // DBUS interface.
    void viewCreated(uint id); //XXX this is actually a WId but qdbuscpp2xml is dumb
    void hidden();

public Q_SLOTS:
    // DBUS interface.
    void showPlasma();
    void hidePlasma();
    //not really slots, but we want them in dbus
    uint viewWinId();

private Q_SLOTS:
    void cleanup();
    void createView(Plasma::Containment *containment);
    void adjustSize(int screen);
    void dialogDestroyed(QObject *obj);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);

    Plasma::Corona *m_corona;
    SaverView *m_view;
    QList<QWidget*> m_dialogs;
    bool m_cheats;
};

#endif // multiple inclusion guard
