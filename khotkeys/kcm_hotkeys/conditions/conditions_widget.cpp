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

#include "conditions/conditions_widget.h"
#include "conditions/conditions_list_base.h"
#include "conditions/conditions_list.h"
#include "conditions/condition.h"
#include "conditions/conditions.h"
#include "conditions/conditions_visitor.h"
#include "conditions/existing_window_condition.h"
#include "conditions/active_window_condition.h"

#include "windows_helper/window_selection_list.h"

#include "helper_widgets/window_definition_list_widget.h"

#include "condition_type_menu.h"

#include <QtCore/QStack>

#include <KDE/KDebug>

class BuildTree : public KHotKeys::ConditionsVisitor
    {
public:

    BuildTree(QTreeWidget *tree);

    void build();

    virtual void visitConditionsListBase(KHotKeys::Condition_list_base *list);
    virtual void visitConditionsList(KHotKeys::Condition_list *list);
    virtual void visitCondition( KHotKeys::Condition *condition );

    QMap<QTreeWidgetItem*, KHotKeys::Condition*> _items;
private:

    QTreeWidget *_tree;
    QStack<QTreeWidgetItem*> _stack;
    };


BuildTree::BuildTree( QTreeWidget *tree )
    :   _tree(tree)
    {
    _stack.push(_tree->invisibleRootItem());
    }


void BuildTree::visitCondition(KHotKeys::Condition *cond)
    {
    QTreeWidgetItem *item = new QTreeWidgetItem(_stack.top());
    item->setText(0, cond->description());
    _items.insert(item, cond);
    }


void BuildTree::visitConditionsList(KHotKeys::Condition_list *list)
    {
    Q_ASSERT(_stack.count()==1);

    QTreeWidgetItem *parent = _stack.top();

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, i18nc("Add a new condition", "And"));
    _items.insert(item, list);
    _stack.push(item);

    for(KHotKeys::Condition_list_base::Iterator it = list->begin();
            it != list->end();
            ++it)
        {
        (*it)->visit(this);
        }

    _tree->expandAll();
    }


void BuildTree::visitConditionsListBase(KHotKeys::Condition_list_base *list)
    {
    QTreeWidgetItem *parent = _stack.top();

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, list->description());
    _items.insert(item, list);
    _stack.push(item);

    for(KHotKeys::Condition_list_base::Iterator it = list->begin();
            it != list->end();
            ++it)
        {
        // QTreeWidgetItem *child = new QTreeWidgetItem(item);
        // child->setText(0, (*it)->description());
        // _items.insert(child, list);
        (*it)->visit(this);
        }

    _stack.pop();
    }


ConditionsWidget::ConditionsWidget(QWidget *parent)
    :   QWidget(parent)
        ,_working(NULL)
        ,_changed(false)
    {
    ui.setupUi(this);

    connect(ui.edit_button, SIGNAL(clicked(bool)),
            SLOT(slotEdit()));

    connect(ui.delete_button, SIGNAL(clicked(bool)),
            SLOT(slotDelete()));

    ui.new_button->setMenu(new ConditionTypeMenu(this));
    connect(ui.new_button->menu(), SIGNAL(triggered(QAction*)),
            SLOT(slotNew(QAction*)));
    }


ConditionsWidget::~ConditionsWidget()
    {
    delete _working; _working = NULL;
    }


void ConditionsWidget::copyFromObject()
    {
    Q_ASSERT(_conditions_list);

    // Clear the tree and delete our old working copy
    ui.tree->clear();
    if (_working) delete _working;

    // Create the working copy
    _working = _conditions_list->copy();
    Q_ASSERT(_working->count() == _conditions_list->count());

    kDebug() << _working->count();
    kDebug() << _conditions_list->count();

    // Build the tree
    BuildTree builder(ui.tree);
    _working->visit(&builder);
    _items = builder._items;

    _changed = false;
    }


void ConditionsWidget::copyToObject()
    {
    kDebug();

    Q_ASSERT(_conditions_list);
    if (!_conditions_list) return;

    // Just copy the content from our working copy to the original

    // Remove the old content
    qDeleteAll(*_conditions_list);
    _conditions_list->clear();

    for (KHotKeys::Condition_list::Iterator it = _working->begin();
            it != _working->end();
            ++it)
        {
        kDebug();
        _conditions_list->append((*it)->copy());
        }

    Q_ASSERT(_working->count() == _conditions_list->count());
    _changed = false;
    }


void ConditionsWidget::emitChanged(bool chgd)
    {
    if (_changed == ( chgd || _changed ))
        return;

    // Once changed always changed
    _changed = chgd || _changed;

    emit changed(_changed);
    }


bool ConditionsWidget::hasChanges() const
    {
    return _changed;
    }


void ConditionsWidget::setConditionsList( KHotKeys::Condition_list *list)
    {
    Q_ASSERT(list);
    _conditions_list = list;
    }


void ConditionsWidget::slotDelete()
    {
    QTreeWidgetItem *citem = ui.tree->currentItem();

    // If no item is selected just return
    if (!citem) return;

    // TODO: Ask for confirmation before deleting

    // Get the currently select condition
    KHotKeys::Condition *cond = _items.value(citem);

    // Do not allow deleting the root item
    if (cond==_working) return;

    delete cond;
    delete citem;
    emitChanged(true);
    }


void ConditionsWidget::slotEdit()
    {
    // Get the currently select condition
    QTreeWidgetItem *citem = ui.tree->currentItem();

    // If no item is selected just return
    if (!citem) return;

    KHotKeys::Condition *cond = _items.value(citem);

    // Currently we only allow editing existing and active window conditions.
    // TODO: Disable buttons according to active item

    KHotKeys::Existing_window_condition *ewcond = 
        dynamic_cast<KHotKeys::Existing_window_condition*>(cond);
    if (ewcond)
        {
        WindowDefinitionListDialog dialog(ewcond->window());
        switch (dialog.exec())
            {
            case QDialog::Accepted:
                {
                citem->setText(0, ewcond->description());
                emitChanged(true);
                }
                break;

            case QDialog::Rejected:
                // Nothing to do
                return;

            default:
                Q_ASSERT(false);
                return;
            }
        }

    KHotKeys::Active_window_condition *awcond = 
        dynamic_cast<KHotKeys::Active_window_condition*>(cond);
    if (awcond)
        {
        WindowDefinitionListDialog dialog(awcond->window());
        switch (dialog.exec())
            {
            case QDialog::Accepted:
                {
                citem->setText(0, awcond->description());
                emitChanged(true);
                }
                break;

            case QDialog::Rejected:
                // Nothing to do
                return;

            default:
                Q_ASSERT(false);
                return;
            }
        }

    return;
    }


void ConditionsWidget::slotNew(QAction* action)
    {
    QTreeWidgetItem *citem = ui.tree->currentItem();

    KHotKeys::Condition *cond;
    if (!citem)
        {
        // If no item is selected create the new condition as a top level
        // condition
        cond = _working;
        citem = ui.tree->invisibleRootItem()->child(0);
        Q_ASSERT(citem);
        }
    else
        {
        // Get the currently select condition
        cond = _items.value(citem);
        }

    // get the nearest list
    KHotKeys::Condition_list_base *parent = dynamic_cast<KHotKeys::Condition_list_base*>(cond);

    if (!parent)
        {
        parent = cond->parent();
        citem = citem->parent();
        }

    Q_ASSERT(parent);

    switch (action->data().toInt())
        {
        case ConditionTypeMenu::ACTIVE_WINDOW:
            {
            KHotKeys::Windowdef_list *list = new KHotKeys::Windowdef_list();
            WindowDefinitionListDialog dialog(list);
            switch (dialog.exec())
                {
                case QDialog::Accepted:
                    {
                    KHotKeys::Active_window_condition *cond =
                        new KHotKeys::Active_window_condition(list, parent);
                    QTreeWidgetItem *item = new QTreeWidgetItem(citem);
                    item->setText(0, cond->description());
                    _items.insert(item, cond);
                    }
                    break;

                case QDialog::Rejected:
                    delete list;
                    return;

                default:
                    Q_ASSERT(false);
                    delete list;
                    return;
                }
            }
            break;

        case ConditionTypeMenu::EXISTING_WINDOW:
            {
            KHotKeys::Windowdef_list *list = new KHotKeys::Windowdef_list();
            WindowDefinitionListDialog dialog(list);
            switch (dialog.exec())
                {
                case QDialog::Accepted:
                    {
                    KHotKeys::Existing_window_condition *cond =
                        new KHotKeys::Existing_window_condition(list, parent);
                    QTreeWidgetItem *item = new QTreeWidgetItem(citem);
                    item->setText(0, cond->description());
                    _items.insert(item, cond);
                    }
                    break;

                case QDialog::Rejected:
                    delete list;
                    return;

                default:
                    Q_ASSERT(false);
                    delete list;
                    return;
                }
            }
            break;

        case ConditionTypeMenu::AND:
            {
            KHotKeys::And_condition *cond = new KHotKeys::And_condition(parent);
            QTreeWidgetItem *item = new QTreeWidgetItem(citem);
            item->setText(0, cond->description());
            _items.insert(item, cond);
            }
            break;

        case ConditionTypeMenu::OR:
            {
            KHotKeys::Or_condition *cond = new KHotKeys::Or_condition(parent);
            QTreeWidgetItem *item = new QTreeWidgetItem(citem);
            item->setText(0, cond->description());
            _items.insert(item, cond);
            }
            break;

        case ConditionTypeMenu::NOT:
            {
            KHotKeys::Not_condition *cond = new KHotKeys::Not_condition(parent);
            QTreeWidgetItem *item = new QTreeWidgetItem(citem);
            item->setText(0, cond->description());
            _items.insert(item, cond);
            }
            break;

        default:
            Q_ASSERT(false);
            break;
        }

    emitChanged(true);
    }

#include "moc_conditions_widget.cpp"
