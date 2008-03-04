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

#ifndef MENUVIEW_H
#define MENUVIEW_H

// Qt
#include <QModelIndex>

// KDE
#include <KMenu>

class QAbstractItemModel;

namespace Kickoff
{

class UrlItemLauncher;

/**
 * A view for a QAbstractItemModel which displays the model (set with setModel())
 * as a hierarchical menu.
 *
 * When the menu is executed and an item is triggered, the model index associated with the 
 * chosen item can be found by calling indexForAction() with the triggered action.  The action
 * associated with a particular model index can be found using actionForIndex().
 *
 * MenuView creates actions for parts of the model on demand as the user explores the menu.
 * The type of action created for leaf items in the tree can be changed by re-implementing
 * createLeafAction().  When a new action is created or if the corresponding model
 * index's data changes, updateAction() is called to set the action's properties.  This
 * can be reimplemented in sub-classes to change the appearance of the actions.
 */ 
class MenuView : public KMenu 
{
Q_OBJECT
public:

    /** Constructs a new menu with the specified @p parent */
    MenuView(QWidget *parent = 0);
    virtual ~MenuView();

    /** Sets the model displayed by this menu. */
    void setModel(QAbstractItemModel *model);
    /** Returns the model displayed by this menu. */
    QAbstractItemModel *model() const;

    /** Returns the UrlItemLauncher used to handle launching of urls. */
    UrlItemLauncher *launcher() const;

    /** Maps an action in the menu to its corresponding index in model() */
    QModelIndex indexForAction(QAction *action) const;
    /** 
     * Maps an index in the model to its corresponding action in the menu. 
     * If @p index is invalid then menuAction() will be returned.  If @p index
     * is in a part of the tree which the user has not yet explored then 0 will
     * be returned because the menu hierarchy is constructed on-demand as the user
     * explores the menu. 
     */
    QAction *actionForIndex(const QModelIndex& index) const;

    /** Sets the column from the model which is used to construct the actions in the menu. */
    void setColumn(int column);
    /** See setColumn() */
    int column() const;

    /** The format type enumeration. */
    enum FormatType {
        Name = 0, ///< Name only
        Description, ///< Description only
        NameDescription, ///< Name (Description)
        DescriptionName ///< Description (Name)
    };
    /** \return the format type. */
    FormatType formatType() const;
    /** Set the format type. */
    void setFormatType(FormatType formattype);

protected:
    /** 
     * Creates a new action to represent a leaf index in the tree.  A leaf index
     * is one which does not have children.  The default implementation creates a new
     * QAction with no properties set.  updateAction() is immediately called on the 
     * return action to set its text and icon.
     *
     * @param index The index in the model for which an action should be created
     * @param parent The object which should be set as the parent of the new action
     */
    virtual QAction *createLeafAction(const QModelIndex& index,QObject *parent);
    /** 
     * Sets the text, icon and other properties of @p action using the data 
     * associated with @p index in the model().  This is called whenever the data for
     * a range of indexes in the tree is altered. 
     *
     * The default implementation sets the action's text to the index's Qt::DisplayRole data
     * and the action's icon to the index's Qt::DecorationRole data.
     */
    virtual void updateAction(QAction *action, const QModelIndex& index);

    //Reimplemented
    virtual bool eventFilter(QObject * watched, QEvent *event);

public Q_SLOTS:
    // an item in the menu got triggered
    void actionTriggered(QAction* action);

private Q_SLOTS:
    void rowsInserted(const QModelIndex& parent,int start,int end);
    void rowsRemoved(const QModelIndex& parent,int start,int end);
    void dataChanged(const QModelIndex& topLeft,const QModelIndex& bottomRight);
    void modelReset();
    // performs on-demand filling of sub-menus in the tree
    void fillSubMenu();

private:
    class Private;
    Private * const d;
};

}

#endif // MENUVIEW_H

