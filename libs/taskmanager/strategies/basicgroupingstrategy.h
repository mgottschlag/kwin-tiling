/*****************************************************************

Copyright 2008 Christian Mollekopf <robertknight@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef BASICGROUPINGSTRATEGY_H
#define BASICGROUPINGSTRATEGY_H

#include <taskmanager/abstractgroupingstrategy.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{
/**
 * Returns Basic guesses, not very advanced
 */
class TASKMANAGER_EXPORT BasicGroupingStrategy: public AbstractGroupingStrategy
{
    Q_OBJECT

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
