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

#ifndef ABSTRACTGROUPINGSTRATEGY_H
#define ABSTRACTGROUPINGSTRATEGY_H

/**
 * Base class for strategies which can be used to
 * automatically group tasks.
 */
class AbstractGroupingStrategy
{
public:
    virtual ~AbstractGroupingStrategy() {};

    /**
     * Specifies a suggested grouping of tasks
     */
    class GroupSuggestion
    {
    public:
        /**
         * Constructs a new GroupSuggestion for @p tasks
         * with a suggested @p name
         */
        GroupSuggestion(const QString &name,
                        const QSet<AbstractTaskItem*> &tasks);

        /** A suggested name for the group. */
        QString name() const;
        /** The tasks to group. */
        QSet<AbstractTaskItem*> tasks() const;
    private:
        QString _name;
        QSet<AbstractTaskItem*> _tasks;
    };

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
    virtual QList<GroupSuggestion> suggestGroups(const QSet<AbstractTaskItem*> tasks) = 0;
};

#endif
