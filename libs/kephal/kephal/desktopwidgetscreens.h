/*
Copyright 2010 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DESKTOPWIDGETSCREENS_H
#define DESKTOPWIDGETSCREENS_H

#include "screens.h"

#include <QMap>


namespace Kephal
{

class SimpleScreen;

class DesktopWidgetScreens : public Screens
{
Q_OBJECT
    public:
    DesktopWidgetScreens(QObject * parent = 0);
    virtual ~DesktopWidgetScreens();
    /**
     * @reimp Screens
     */
    QList<Screen *> screens();
protected Q_SLOTS:
    void handleScreenCountChanged(int);
    void handleScreenResized(int);
private:
    QList<Kephal::SimpleScreen *> m_screens;
};

} // namespace Kephal

#endif // DESKTOPWIDGETSCREENS_H
