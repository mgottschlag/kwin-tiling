/***************************************************************************
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ACTIVITYBAR_H
#define ACTIVITYBAR_H


#include <plasma/applet.h>

namespace Plasma
{
    class TabBar;
    class Containment;
    class View;
}

class ActivityBar : public Plasma::Applet
{
Q_OBJECT
public:
    ActivityBar(QObject *parent, const QVariantList &args);
    ~ActivityBar();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);

private Q_SLOTS:
    void switchContainment(int newActive);
    void containmentAdded(Plasma::Containment *containment);
    void containmentDestroyed(QObject *obj);
    void screenChanged(int wasScreen, int isScreen, Plasma::Containment *containment);
    void contextChanged(Plasma::Context *context);

private:
    int m_activeContainment;
    Plasma::View *m_view;
    Plasma::TabBar *m_tabBar;
    QList<Plasma::Containment*> m_containments;
};


K_EXPORT_PLASMA_APPLET(activitybar, ActivityBar)
#endif
