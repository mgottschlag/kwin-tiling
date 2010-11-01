/***************************************************************************
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2010 by Chani Armitage <chanika@gmail.com>              *
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


#include <Plasma/Applet>
#include <Plasma/DataEngine>

namespace Plasma
{
    class TabBar;
    class Containment;
}

class ActivityBar : public Plasma::Applet
{
Q_OBJECT
public:
    ActivityBar(QObject *parent, const QVariantList &args);
    ~ActivityBar();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    //insert in m_containments, keeping it ordered by id()
    void insertContainment(Plasma::Containment *cont);
    void insertActivity(const QString &id);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void switchContainment(int newActive);
    void containmentAdded(Plasma::Containment *containment);
    void containmentDestroyed(QObject *obj);
    void screenChanged(int wasScreen, int isScreen, Plasma::Containment *containment);
    void contextChanged(Plasma::Context *context);
    void currentDesktopChanged(const int currentDesktop);

    void switchActivity(int newActive);
    void activityAdded(const QString &id);
    void activityRemoved(const QString &id);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

private:
    Plasma::TabBar *m_tabBar;
    QList<Plasma::Containment*> m_containments;
    QStringList m_activities;
    Plasma::DataEngine *m_engine;
};


K_EXPORT_PLASMA_APPLET(activitybar, ActivityBar)
#endif
