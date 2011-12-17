/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008-2009 Sebastian Sauer <mail@dipe.org>

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
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QStack>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QStandardItem>
#include <QtGui/QStyleOptionMenuItem>
#include <QtGui/QPainter>
#include <QtGui/QToolTip>

// KDE
#include <KDebug>
#include <KUrl>
#include <KIconLoader>

// Local
#include "core/models.h"
#include "core/itemhandlers.h"

#define MAX_NAME_SIZE 50

#ifndef KDE_USE_FINAL
Q_DECLARE_METATYPE(QPersistentModelIndex)
#endif
Q_DECLARE_METATYPE(QAction*)

using namespace Kickoff;

/// @internal d-pointer class
class MenuView::Private
{
public:
    enum { ActionRole = Qt::UserRole + 52 };

    Private(MenuView *q) : q(q), column(0), launcher(new UrlItemLauncher(q)), formattype(MenuView::DescriptionName) {}
    ~Private()
    {
        qDeleteAll(items);
    }

    QAction *createActionForIndex(QAbstractItemModel *model, const QModelIndex& index, QMenu *parent)
    {
        Q_ASSERT(index.isValid());
        QAction *action = 0;
        if (model->hasChildren(index)) {
            KMenu *childMenu = new KMenu(parent);
            childMenu->installEventFilter(q);
            childMenu->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(childMenu, SIGNAL(customContextMenuRequested(QPoint)),
                    q, SLOT(contextMenuRequested(QPoint)));
            action = childMenu->menuAction();
            buildBranch(childMenu, model, index);
        } else {
            action = q->createLeafAction(index, parent);
        }

        q->updateAction(model, action, index);
        return action;
    }

    QString trunctuateName(QString name, int maxSize)
    {
        if (name.length() <= maxSize) {
            return name;
        }

        maxSize -= 2; // remove the 3 placeholder points
        const int start = maxSize / 3; //use one third of the chars for the start of the name
        const int end = maxSize - start;
        return name.left(start) + ".." + name.right(end);
    }

    void buildBranch(KMenu *menu, QAbstractItemModel *model, const QModelIndex& parent)
    {
        if (model->canFetchMore(parent)) {
            model->fetchMore(parent);
        }
        const int rowCount = model->rowCount(parent);
        for (int i = 0; i < rowCount; i++) {
            QAction *action = createActionForIndex(model, model->index(i, column, parent), menu);
            action->setText(trunctuateName(action->text(), MAX_NAME_SIZE));
            menu->addAction(action);
        }
    }

    QModelIndex findByRelPath(QAbstractItemModel * model, const QModelIndex & parent, const QString & relPath)
    {
        QModelIndex newRoot;
        if (model->canFetchMore(parent)) {
            model->fetchMore(parent);
        }
        const int rowCount = model->rowCount(parent);
        for (int row = 0; row < rowCount; row++) {
            QModelIndex index = model->index(row, 0, parent);
            Q_ASSERT(index.isValid());
            if (index.data(Kickoff::RelPathRole).isValid()) {
                QString indexRelPath = index.data(Kickoff::RelPathRole).toString();
                if (relPath == indexRelPath) {
                    newRoot = index;
                    break;
                }
                if (!indexRelPath.isEmpty() && relPath.startsWith(indexRelPath)) {
                    newRoot = findByRelPath(model, index, relPath);
                    break;
                }
            }
        }
        return newRoot;
    }


    MenuView * const q;
    int column;
    UrlItemLauncher *launcher;
    MenuView::FormatType formattype;
    QPoint mousePressPos;
    QList<QStandardItem*> items;
    QHash<QAbstractItemModel*, QAction*> modelsHeader;
};

MenuView::MenuView(QWidget *parent, const QString &title, const QIcon &icon)
    : KMenu(parent)
    , d(new Private(this))
{
    if (! title.isNull())
        setTitle(title);
    if (! icon.isNull())
        setIcon(icon);

    installEventFilter(this);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));
}

MenuView::~MenuView()
{
    QHashIterator<QAbstractItemModel*, QAction *> it(d->modelsHeader);
    while (it.hasNext()) {
        it.next();
        it.key()->disconnect(this);
    }

    delete d;
}

QAction *MenuView::createLeafAction(const QModelIndex &index, QObject *parent)
{
    Q_UNUSED(index);
    return new QAction(parent);
}

void MenuView::updateAction(QAbstractItemModel *model, QAction *action, const QModelIndex& index)
{
    bool isSeparator = index.data(Kickoff::SeparatorRole).value<bool>();

    // if Description or DescriptionName -> displayOrder = Kickoff::NameAfterDescription
    //    Qt::DisplayRole returns genericName, the generic name e.g. "Advanced Text Editor" or "Spreadsheet" or just "" (right, it's a mess too)
    //    Kickoff::SubTitleRole returns appName, the name  e.g. "Kate" or "OpenOffice.org Calc" (right, sometimes the text is also used for the generic app-name)
    //
    // if Name or NameDescription or NameDashDescription -> displayOrder = Kickoff::NameBeforeDescription
    //    Qt::DisplayRole returns appName,
    //    Kickoff::SubTitleRole returns genericName.

    QString mainText = index.data(Qt::DisplayRole).value<QString>().replace('&', "&&");
    QString altText = index.data(Kickoff::SubTitleRole).value<QString>().replace('&', "&&");
    if (action->menu() != 0) { // if it is an item with sub-menuitems, we probably like to thread them another way...
        action->setText(mainText);
    } else {
        switch (d->formattype) {
        case Name:
        case Description: {
            action->setText(mainText);
            action->setToolTip(altText);
        } break;
        case NameDescription: // fall through
        case NameDashDescription: // fall through
        case DescriptionName: {
            if (!mainText.isEmpty()) { // seems we have a program, but some of them don't define a name at all
                if (mainText.contains(altText, Qt::CaseInsensitive)) { // sometimes the description contains also the name
                    action->setText(mainText);
                } else if (altText.contains(mainText, Qt::CaseInsensitive)) { // and sometimes the name also contains the description
                    action->setText(altText);
                } else { // seems we have a perfect desktop-file (likely a KDE one, heh) and name+description are clear separated
                    if (d->formattype == NameDashDescription) {
                        action->setText(QString("%1 - %2").arg(mainText).arg(altText));
                    } else {
                        action->setText(QString("%1 (%2)").arg(mainText).arg(altText));
                    }
                }
            } else { // if there is no name, let's just use the describing text
                action->setText(altText);
            }
        } break;
        }
    }

    action->setSeparator(isSeparator);
    if (!isSeparator) {
        action->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
    }

    // we map modelindex and action together
    action->setData(qVariantFromValue(QPersistentModelIndex(index)));

    // don't emit the dataChanged-signal cause else we may end in a infinite loop
    model->blockSignals(true);
    model->setData(index, qVariantFromValue(action), Private::ActionRole);
    model->blockSignals(false);
}

bool MenuView::eventFilter(QObject *watched, QEvent *event)
{
    switch(event->type()) {
    case QEvent::MouseMove: {
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

            QUrl url = index.data(UrlRole).toUrl();
            if (url.isEmpty()) {
                return KMenu::eventFilter(watched, event);
            }

            QMimeData *mimeData = new QMimeData();
            mimeData->setUrls(QList<QUrl>() << url);
            mimeData->setText(url.toString());
            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);

            QIcon icon = action->icon();
            drag->setPixmap(icon.pixmap(IconSize(KIconLoader::Desktop)));

            d->mousePressPos = QPoint();

            Qt::DropAction dropAction = drag->exec();
            Q_UNUSED(dropAction);

            return true;
        }

        break;
    }

    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QMenu *watchedMenu = qobject_cast<QMenu*>(watched);
        if (watchedMenu) {
            d->mousePressPos = mouseEvent->pos();
        }

        break;
    }

    case QEvent::MouseButtonRelease: {
        QMenu *watchedMenu = qobject_cast<QMenu*>(watched);
        if (watchedMenu) {
            d->mousePressPos = QPoint();
        }

        break;
    }

    case QEvent::Hide: {
        emit afterBeingHidden();
        break;
    }

    case QEvent::ToolTip: {
        bool hide = true;

        if ((d->formattype == Name) || (d->formattype == Description)) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            QMenu *watchedMenu = qobject_cast<QMenu*>(watched);

            if (watchedMenu && watchedMenu->activeAction()) {
                QString toolTip = watchedMenu->activeAction()->toolTip();

                if ((toolTip != watchedMenu->activeAction()->text()) && (watchedMenu->activeAction()->menu() == 0)) {
                QToolTip::showText(helpEvent->globalPos(), toolTip);
                hide = false ;
                }
            }
        }

        if (hide) {
            QToolTip::hideText();
            event->ignore();
        }

        break;
    }

    default:
        break;
    }

    return KMenu::eventFilter(watched, event);
}

void MenuView::addModel(QAbstractItemModel *model, MenuView::ModelOptions options, const QString & relativePath)
{
    QString title = model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString();

    QAction *header = addTitle(title);
    header->setVisible(false);

    d->modelsHeader.insert(model, header);

    if (options & MergeFirstLevel) {
        const int count = model->rowCount();
        for (int row = 0; row < count; ++row) {
            QModelIndex index = model->index(row, 0, QModelIndex());
            Q_ASSERT(index.isValid());

            const QString title = index.data(Qt::DisplayRole).value<QString>();
            if (count > 1 && ! title.isEmpty() && model->rowCount(index) > 0) {
                addTitle(title);
            }

            model->blockSignals(true);
            model->setData(index, qVariantFromValue(this->menuAction()), Private::ActionRole);
            model->blockSignals(false);

            d->buildBranch(this, model, index);
        }
    } else {
        QModelIndex root;
        if (!relativePath.isEmpty()) {
            root = d->findByRelPath(model, root, relativePath);
        }
        d->buildBranch(this, model, root);
    }

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(model, SIGNAL(modelReset()), this, SLOT(modelReset()));
}

void MenuView::addItem(QStandardItem *item)
{
    QAction *action = new QAction(item->icon(), item->text(), this);
    KUrl url(item->data(Kickoff::UrlRole).toString());
    Q_ASSERT(url.isValid());
    action->setData(url);
    addAction(action);
    d->items << item;
}

UrlItemLauncher *MenuView::launcher() const
{
    return d->launcher;
}

QModelIndex MenuView::indexForAction(QAction *action) const
{
    Q_ASSERT(action != 0);
    QPersistentModelIndex index = action->data().value<QPersistentModelIndex>();
    return index;
}

QAction *MenuView::actionForIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return this->menuAction();
    }

    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);
    QVariant v = model->data(index, Private::ActionRole);
    Q_ASSERT(v.isValid());
    QAction* a = v.value<QAction*>();
    Q_ASSERT(a);
    return a;
}

bool MenuView::isValidIndex(const QModelIndex& index) const
{
    QVariant v = (index.isValid() && index.model()) ? index.model()->data(index, Private::ActionRole) : QVariant();
    return v.isValid() && v.value<QAction*>();
}

void MenuView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    kDebug() << start << end;

    Q_ASSERT(parent.isValid());
    Q_ASSERT(parent.model());

    //Q_ASSERT( ! isValidIndex(parent) );
    QMenu *menu = isValidIndex(parent) ? actionForIndex(parent)->menu() : this;

    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(parent.model());
    if (!model) {
        return;
    }

    QList<QAction*> newActions;
    for (int row = start; row <= end; row++) {
        QModelIndex index = model->index(row, d->column, parent);
        QAction *newAction = d->createActionForIndex(model, index, menu);
        kDebug()<<"new action="<<newAction->text();
        newActions << newAction;
    }

    int lastidx = -1, offset = -1;
    for (int i = 0; i < menu->actions().count(); ++i) {
        QAction *action = menu->actions()[i];
        Q_ASSERT(action);
        QModelIndex index = indexForAction(action);
        if (index.isValid() && index.model() == model) {
            lastidx = i;
            if(++offset == start)
                break;
        }
    }

    if (lastidx < 0 && d->modelsHeader.contains(model)) {
        QAction *header = d->modelsHeader[model];
        lastidx = menu->actions().indexOf(header);
    }

    if (lastidx >= 0) {
        if (offset < start) {
            lastidx++; // insert action the item right after the last valid index
        }

        if (lastidx < menu->actions().count()) {
            menu->insertActions(menu->actions()[lastidx], newActions);
        } else {
            lastidx = -1;
        }
    }

    if (lastidx < 0) {
        // just append the action
        menu->addActions(newActions);
    }
}

void MenuView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    kDebug() << start << end;
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    modelReset();
}

void MenuView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(topLeft.model());
    Q_ASSERT(model);

    //Q_ASSERT( ! isValidIndex(topLeft) );
    QMenu *menu = isValidIndex(topLeft) ? actionForIndex(topLeft)->menu() : this;

    QList<QAction*> actions = menu->actions();
    Q_ASSERT(bottomRight.row() < actions.count());

    for (int row = topLeft.row(); row <= bottomRight.row() && row < actions.count(); ++row) {
        QModelIndex index = model->index(row, d->column, topLeft.parent());
        kDebug()<<row<<index.data(Qt::DisplayRole).value<QString>();
        updateAction(model, actions[row], index);
    }
}

void MenuView::modelReset()
{
    kDebug();
    deleteLater(); // we need to force clearance of the menu and rebuild from scratch
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

void MenuView::setModelTitleVisible(QAbstractItemModel *model, bool visible)
{
    if (d->modelsHeader.contains(model)) {
        QAction *header = d->modelsHeader[model];
        header->setVisible(visible);
    }
}

void MenuView::actionTriggered(QAction *action)
{
    KUrl url = action->data().value<KUrl>();
    if (url.isValid()) {
        d->launcher->openUrl(url.url());
    } else {
        QModelIndex index = indexForAction(action);
        if (index.isValid()) {
            d->launcher->openItem(index);
        } else {
            kWarning()<<"Invalid action objectName="<<action->objectName()<<"text="<<action->text()<<"parent="<<(action->parent()?action->parent()->metaObject()->className():"NULL");
        }
    }
}

void MenuView::contextMenuRequested(const QPoint &pos)
{
    KMenu *menu = qobject_cast<KMenu *>(sender());
    emit customContextMenuRequested(menu, pos);
}

#include "menuview.moc"
