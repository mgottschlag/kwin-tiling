/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "menuview.h"

// Qt
#include <QtCore/QAbstractItemModel>
#include <QtCore/QStack>
#include <QtCore/QUrl>
#include <QtDebug>

// Local
#include "core/models.h"
#include "core/itemhandlers.h"

using namespace Kickoff;

class MenuView::Private
{
public:
    Private(MenuView *parent) : q(parent) , model(0) , column(0), launcher(new UrlItemLauncher(parent)) {}

    QAction *createActionForIndex(const QModelIndex& index,QWidget *parent)
    {
        Q_ASSERT(index.isValid());

        QAction *action = 0; 
   
        if (model->hasChildren(index)) {
            QMenu *childMenu = new QMenu(parent);
            QObject::connect(childMenu,SIGNAL(aboutToShow()),q,SLOT(fillSubMenu()));
            action = childMenu->menuAction();
        } else {
            action = q->createLeafAction(index,parent);
        }

        q->updateAction(action,index);

        return action;
    }
    
    void buildBranch(QMenu *menu,const QModelIndex& parent)
    {
        int rowCount = model->rowCount(parent);
        for (int i=0;i<rowCount;i++) {
            QAction *action = createActionForIndex(model->index(i,column,parent),menu);
            menu->addAction(action);
        } 
    }

    MenuView * const q;
    QAbstractItemModel *model;
    int column;
    UrlItemLauncher *launcher;
};

MenuView::MenuView(QWidget *parent)
    : KMenu(parent)
    , d(new Private(this))
{
    connect(this,SIGNAL(triggered(QAction*)),this,SLOT(actionTriggered(QAction*)));
}
MenuView::~MenuView()
{
    delete d;
}
QAction *MenuView::createLeafAction(const QModelIndex&,QObject *parent)
{
    return new QAction(parent); 
}
void MenuView::updateAction(QAction *action,const QModelIndex& index)
{
    QString text = index.data(Qt::DisplayRole).value<QString>();
    const QString name = index.data(Kickoff::SubTitleRole).value<QString>();
    if (action->menu()==0 && name.contains(text,Qt::CaseInsensitive))
        text = name;
    action->setText(text.replace("&","&&"));

    action->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
}
void MenuView::setModel(QAbstractItemModel *model)
{
    d->model = model;
    clear();
    if (d->model) {
        d->buildBranch(this,QModelIndex());
    }
}
QAbstractItemModel *MenuView::model() const
{
    return d->model;
}
UrlItemLauncher *MenuView::launcher() const
{
    return d->launcher;
}
QModelIndex MenuView::indexForAction(QAction *action) const
{
    Q_ASSERT(d->model);
    Q_ASSERT(action != 0);

    QStack<int> rows;
 
    // find the menu containing the action.  for leaf actions this is the 
    // action's parent widget.  for actions that are sub-menus this is the
    // action's parent widget's parent. 
    QWidget *parentWidget = action->parentWidget();
    if (action->menu() != 0) {
        parentWidget = parentWidget->parentWidget();
    }

    // navigate up the menu hierarchy to find out the position of each
    // action on the path to the specified action 
    QMenu *menu = qobject_cast<QMenu*>(parentWidget);
    while (menu) {
        int row = menu->actions().indexOf(action);
        if( row < 0 )
            return QModelIndex();
        rows.push(row);

        if (menu == this) {
            break;
        }
        action = menu->menuAction();
        menu = qobject_cast<QMenu*>(menu->parentWidget());
    }

    // navigate down the model using the row information from the QMenu traversal
    // to get the index for the specified action
    QModelIndex index;
    while (!rows.isEmpty()) {
        index = d->model->index(rows.pop(),d->column,index);
    }

    return index;
}
QAction *MenuView::actionForIndex(const QModelIndex& index) const
{
    Q_ASSERT(d->model);
   
    if (!index.isValid()) {
        return this->menuAction(); 
    } 

    // navigate up the model to get the rows of each index along the path
    // to the specified index
    QStack<int> rows;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        rows.push(parent.row());
        parent = parent.parent();
    }

    // navigate down the menu using the row information from the model 
    // traversal to find the action for the specified index
    const QMenu *menu = this;
    while (!rows.isEmpty()) {
       if (menu->actions().isEmpty()) {
            // if we reach an empty menu along the way this means that the index
            // is in part of the tree for which the menu hierarchy has not been constructed
            // because the user hasn't browsed there yet 
            return 0;
       }

       menu = menu->actions()[rows.pop()]->menu(); 
    }
    return menu->actions()[index.row()];
}
void MenuView::rowsInserted(const QModelIndex& parent,int start,int end)
{
    QAction *menuAction = actionForIndex(parent);
    if (!menuAction) {
        return;
    }
    QMenu *menu = menuAction->menu();

    Q_ASSERT(menu);

    QList<QAction*> newActions;
    for (int row = start; row <= end; row++) {
        QAction *newAction = d->createActionForIndex(d->model->index(row,d->column,parent),menu);
        newActions << newAction;
    }

    Q_ASSERT(menu->actions().count() > start);
    insertActions(menu->actions()[start],newActions);
}
void MenuView::rowsRemoved(const QModelIndex& parent,int start,int end)
{
    QAction *menuAction = actionForIndex(parent);
    if (!menuAction) {
        return;
    }
    QMenu *menu = menuAction->menu();

    Q_ASSERT(menu);

    QList<QAction*> actions = menu->actions();
    for (int row = start; row <= end; row++) {
        menu->removeAction(actions[row]);
    }
}
void MenuView::dataChanged(const QModelIndex& topLeft,const QModelIndex& bottomRight)
{
    QAction *menuAction = actionForIndex(topLeft.parent());
    if (!menuAction) {
        return;
    }
    QMenu *menu = menuAction->menu();

    QList<QAction*> actions = menu->actions();
    for (int row=topLeft.row(); row <= bottomRight.row(); row++) {
        updateAction(actions[row],d->model->index(row,d->column,topLeft.parent()));
    }
}
void MenuView::modelReset()
{
    // force clearance of the menu
    setModel(0); 
    // rebuild the menu from scratch
    setModel(d->model);
}
void MenuView::fillSubMenu()
{
    QMenu *subMenu = qobject_cast<QMenu*>(sender());
    Q_ASSERT(subMenu);
    Q_ASSERT(subMenu->isEmpty());

    QModelIndex menuIndex = indexForAction(subMenu->menuAction());
    Q_ASSERT(menuIndex.isValid());

    if (d->model->canFetchMore(menuIndex)) {
        d->model->fetchMore(menuIndex);
    }
    d->buildBranch(subMenu,menuIndex);

    disconnect(sender(),0,this,SLOT(fillSubMenu()));
}
void MenuView::setColumn(int column)
{
    d->column = column;
    modelReset();
}
int MenuView::column() const
{
    return d->column;
}
void MenuView::actionTriggered(QAction *action)
{
    QModelIndex index = indexForAction(action);
    if (index.isValid())
        d->launcher->openItem(index);
}

#include "menuview.moc"
