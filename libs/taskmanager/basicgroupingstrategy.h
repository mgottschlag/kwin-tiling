/*
    Copyright (C) 2007 Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef BASICGROUPINGSTRATEGY_H
#define BASICGROUPINGSTRATEGY_H

#include "abstractgroupingstrategy.h"

namespace TaskManager
{
/**
 * Returns Basic guesses, not very advanced
 */
class KDE_EXPORT BasicGroupingStrategy: public AbstractGroupingStrategy
{
private:
    BasicGroupingStrategy();

public:
    //Get a pointer to the groupingstrategy singleton
   static BasicGroupingStrategy& instance();
    

    /**
     * Examines a set of @p tasks and returns a list of
     * suggested named sub-groups of tasks.
     *
     * The suggested groups may include all, some or none from the
     * @p tasks set.
     *
     * Sub-classes must re-implement this method to arrange tasks
     * into groups according to various criteria.
     */
    QList<AbstractGroupingStrategy::GroupSuggestion> suggestGroups(const ItemList &items);
    
    QList<QString> suggestGroupNames(const ItemList &items);
    QList<QColor> suggestGroupColors(const ItemList &items);
    QList<QIcon> suggestGroupIcons(const ItemList &items);

public slots:
    //Get Feedback about what was actually used
    void groupAccepted(const AbstractGroupingStrategy::GroupSuggestion &);
    void nameAccepted(const QString &);
    void colorAccepted(const QColor &);

    //or removed
    void groupRemoved(const AbstractGroupingStrategy::GroupSuggestion &);
    void nameRemoved(const QString &);
    void colorRemoved(const QColor &);

private:
    QList<QString> m_usedNames;
    QList<QColor> m_usedColors;
};
}
#endif
