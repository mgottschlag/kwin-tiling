/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008 Sebastian Sauer <mail@dipe.org>

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
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QPersistentModelIndex>

// KDE
#include <KDebug>
#include <KUrl>
#include <KIconLoader>

// Local
#include "core/models.h"
#include "core/itemhandlers.h"

Q_DECLARE_METATYPE(QPersistentModelIndex)
Q_DECLARE_METATYPE(QAction*)

using namespace Kickoff;

/// @internal d-pointer class
class MenuView::Private
{
public:
    enum { ActionRole = Qt::UserRole + 52 };

    Private(MenuView *parent) : q(parent) , model(0) , column(0), launcher(new UrlItemLauncher(parent)), formattype(MenuView::DescriptionName) {}

    QAction *createActionForIndex(const QModelIndex& index, QMenu *parent)
    {
        Q_ASSERT(index.isValid());

        QAction *action = 0; 

        if (model->hasChildren(index)) {
            KMenu *childMenu = new KMenu(parent);
            childMenu->installEventFilter(q);

            action = childMenu->menuAction();

            if (model->canFetchMore(index)) {
                model->fetchMore(index);
            }

            buildBranch(childMenu, index);
        } else {
            action = q->createLeafAction(index,parent);
        }

        q->updateAction(action,index);

        return action;
    }

    void buildBranch(QMenu *menu, const QModelIndex& parent)
    {
        int rowCount = model->rowCount(parent);
        for (int i = 0; i < rowCount; i++) {
            QAction *action = createActionForIndex(model->index(i, column, parent), menu);
            menu->addAction(action);
        }
    }

    MenuView * const q;
    QAbstractItemModel *model;
    int column;
    UrlItemLauncher *launcher;
    MenuView::FormatType formattype;
    QPoint mousePressPos;
};

MenuView::MenuView(QWidget *parent)
    : KMenu(parent)
    , d(new Private(this))
{
    installEventFilter(this);
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
    Q_ASSERT(d->model);

    QString text = index.data(Qt::DisplayRole).value<QString>().replace("&","&&"); // describing text, e.g. "Spreadsheet" or "Rekall" (right, sometimes the text is also used for the generic app-name)
    QString name = index.data(Kickoff::SubTitleRole).value<QString>().replace("&","&&"); // the generic name, e.g. "kspread" or "OpenOffice.org Spreadsheet" or just "" (right, it's a mess too)
    if( action->menu()!=0 ) { // if its an item with sub-menuitems, we probably like to thread them another way...
        action->setText(text);
    }
    else {
        switch( d->formattype ) {
            case Name: {
                if (name.isEmpty()) {
                    action->setText(text);
                } else {
                    action->setText(name);
                }
            } break;
            case Description: {
                if (name.contains(text,Qt::CaseInsensitive)) {
                    text = name;
                }
                action->setText(text);
            } break;
            case NameDescription: // fall through
            case NameDashDescription: // fall through
            case DescriptionName: {
                if (!name.isEmpty()) { // seems we have a program, but some of them don't define a name at all
                    if (text.contains(name,Qt::CaseInsensitive)) { // sometimes the description contains also the name
                        action->setText(text);
                    } else if (name.contains(text,Qt::CaseInsensitive)) { // and sometimes the name also contains the description
                        action->setText(name);
                    } else { // seems we have a perfect desktop-file (likely a KDE one, heh) and name+description are clear separated
                        if (d->formattype == NameDescription) {
                            action->setText(QString("%1 %2").arg(name).arg(text));
                        } else if (d->formattype == NameDashDescription) {
                            action->setText(QString("%1 - %2").arg(name).arg(text));
                        } else {
                            action->setText(QString("%1 (%2)").arg(text).arg(name));
                        }
                    }
                }
                else { // if there is no name, let's just use the describing text
                    action->setText(text);
                }
            } break;
        }
    }

    action->setIcon(index.data(Qt::DecorationRole).value<QIcon>());

    // we map modelindex and action together
    action->setData(qVariantFromValue(QPersistentModelIndex(index)));

    // don't emit the dataChanged-signal cause else we may end in a infinite loop
    d->model->blockSignals(true);
    d->model->setData(index, qVariantFromValue(action), Private::ActionRole);
    d->model->blockSignals(false);
}

bool MenuView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QMenu *watchedMenu = qobject_cast<QMenu*>(watched);
        const int mousePressDistance = !d->mousePressPos.isNull() ? (mouseEvent->pos() - d->mousePressPos).manhattanLength() : 0;

        if (watchedMenu && mouseEvent->buttons() & Qt::LeftButton
            && mousePressDistance >= QApplication::startDragDistance()) {
            QAction *action = watchedMenu->actionAt(mouseEvent->pos());
            if (!action) {
                return KMenu::eventFilter(watched, event);
            }

            QPersistentModelIndex index = action->data().value<QPersistentModelIndex>();
            if (!index.isValid()) {
                return KMenu::eventFilter(watched, event);
            }

            QString urlString = index.data(UrlRole).toString();
            if (urlString.isNull()) {
                return KMenu::eventFilter(watched, event);
            }

            QMimeData *mimeData = new QMimeData();
            mimeData->setData("text/uri-list", urlString.toAscii());
            mimeData->setText(mimeData->text());
            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);

            QIcon icon = action->icon();
            drag->setPixmap(icon.pixmap(IconSize(KIconLoader::Desktop)));

            d->mousePressPos = QPoint();

            Qt::DropAction dropAction = drag->exec();
            Q_UNUSED(dropAction);

            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QMenu *watchedMenu = qobject_cast<QMenu*>(watched);
        if (watchedMenu) {
            d->mousePressPos = mouseEvent->pos();
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMenu *watchedMenu = qobject_cast<QMenu*>(watched);
        if (watchedMenu) {
            d->mousePressPos = QPoint();
        }
    }

    return KMenu::eventFilter(watched, event);
}

void MenuView::setModel(QAbstractItemModel *model)
{
    if (d->model) {
        d->model->disconnect(this);
    }
    d->model = model;
    clear();
    if (d->model) {
        d->buildBranch(this,QModelIndex());
        connect(d->model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
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
    QPersistentModelIndex index = action->data().value<QPersistentModelIndex>();
    Q_ASSERT(index.isValid());
    return index;
}

QAction *MenuView::actionForIndex(const QModelIndex& index) const
{
    Q_ASSERT(d->model);

    if (!index.isValid()) {
        return this->menuAction();
    }

    QVariant v = d->model->data(index, Private::ActionRole);
    Q_ASSERT(v.isValid());
    QAction* a = v.value<QAction*>();
    Q_ASSERT(a);
    return a;
}

bool MenuView::isValidIndex(const QModelIndex& index) const
{
    QVariant v = (d->model && index.isValid()) ? d->model->data(index, Private::ActionRole) : QVariant();
    return v.isValid() && v.value<QAction*>();
}

void MenuView::rowsAboutToBeInserted(const QModelIndex& parent,int start,int end)
{
    if (!isValidIndex(parent)) {
        // can happen if the models data is incomplete yet
        return;
    }

    Q_ASSERT(d->model);

    QAction *menuAction = actionForIndex(parent);
    Q_ASSERT(menuAction);

    QMenu *menu = menuAction->menu();
    Q_ASSERT(menu);

    QList<QAction*> newActions;
    for (int row = start; row <= end; row++) {
        QModelIndex index = d->model->index(row, d->column, parent);
        QAction *newAction = d->createActionForIndex(index, menu);
        newActions << newAction;
    }

    if (start < menu->actions().count()) {
        menu->insertActions(menu->actions()[start],newActions);
    } else {
        menu->addActions(newActions);
    }
}

void MenuView::rowsAboutToBeRemoved(const QModelIndex& parent,int start,int end)
{
    if (!isValidIndex(parent)) {
        // can happen if the models data is incomplete yet
        return;
    }

    Q_ASSERT(d->model);

    QAction *menuAction = actionForIndex(parent);
    Q_ASSERT(menuAction);

    QMenu *menu = menuAction->menu();
    Q_ASSERT(menu);

    QList<QAction*> actions = menu->actions();
    Q_ASSERT(end < actions.count());
    for (int row = end; row >= start; row--) {
        menu->removeAction(actions[row]);
    }
}

void MenuView::dataChanged(const QModelIndex& topLeft,const QModelIndex& bottomRight)
{
    if (!isValidIndex(topLeft.parent())) {
        // can happen if the models data is incomplete yet
        return;
    }

    Q_ASSERT(d->model);

    QAction *menuAction = actionForIndex(topLeft.parent());
    Q_ASSERT(menuAction);

    QMenu *menu = menuAction->menu();
    Q_ASSERT(menu);

    QList<QAction*> actions = menu->actions();
    Q_ASSERT(bottomRight.row() < actions.count());

    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        updateAction(actions[row], d->model->index(row,d->column,topLeft.parent()));
    }
}

void MenuView::modelReset()
{
    // force clearance of the menu and rebuild from scratch
    setModel(d->model);
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

MenuView::FormatType MenuView::formatType() const
{
    return d->formattype;
}

void MenuView::setFormatType(MenuView::FormatType formattype)
{
    d->formattype = formattype;
}

void MenuView::actionTriggered(QAction *action)
{
    Q_ASSERT(d->model);
    QModelIndex index = indexForAction(action);
    Q_ASSERT(index.isValid());
    d->launcher->openItem(index);
}

#include "menuview.moc"
