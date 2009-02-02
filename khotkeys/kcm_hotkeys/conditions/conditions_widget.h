#ifndef CONDITIONS_WIDGET_H
#define CONDITIONS_WIDGET_H
/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtCore/QMap>
#include <QtGui/QWidget>


#include "ui_conditions_widget.h"

class QAction;
class QTreeWidgetItem;

namespace KHotKeys {
    class Condition_list;
    class Condition;
}


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ConditionsWidget : public QWidget
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    ConditionsWidget(QWidget *parent = NULL);

    /**
     * Destructor
     */
    virtual ~ConditionsWidget();

    void copyFromObject();
    void copyToObject();

    void setConditionsList(KHotKeys::Condition_list *list);

    //! Are there uncommited changes?
    bool hasChanges() const;

Q_SIGNALS:

    void changed(bool);

private Q_SLOTS:

    void slotNew(QAction*);
    void slotEdit();
    void slotDelete();

private:

    // Emit the changed(bool) signal if our changed status changes
    void emitChanged(bool);

    //! The original
    KHotKeys::Condition_list *_conditions_list;

    //! The working copy
    KHotKeys::Condition_list *_working;

    //! User Interface Definition
    Ui::ConditionsWidget ui;

    //! Are there uncommited changes?
    bool _changed;

    //! Map between treewidgetitems and conditions
    QMap<QTreeWidgetItem*, KHotKeys::Condition*> _items;

};


#endif /* #ifndef CONDITIONS_WIDGET_H */
