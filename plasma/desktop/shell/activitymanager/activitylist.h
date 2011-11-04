/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITYLIST_H
#define ACTIVITYLIST_H

#include "activityicon.h"
#include "abstracticonlist.h"

namespace KActivities
{
    class Controller;
} // namespace KActivities

namespace Plasma
{
    class Corona;
} // namespace Plasma

class ActivityList : public Plasma::AbstractIconList
{

    Q_OBJECT

public:
    ActivityList(Plasma::Location location = Plasma::BottomEdge, QGraphicsItem *parent = 0);
    ~ActivityList();

protected:
    void updateVisibleIcons();
    void setSearch(const QString &searchString);

private Q_SLOTS:
    void activityAdded(const QString &id);
    void activityRemoved(const QString &id);

    void templateHidden(bool immediate);

    void updateClosable();

private:
    void createActivityIcon(const QString &id);
    void createActivityIcon(const QString &name, const QString &icon, const QString &plugin);

    KActivities::Controller *m_activityController;
    ActivityIcon * m_scheduleHideOnAdd;

};

#endif //APPLETSLIST_H
