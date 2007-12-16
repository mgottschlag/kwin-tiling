/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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
#include "ui/launcher.h"

// Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

// KDE
#include <KLocalizedString>
#include <KIcon>
#include <solid/device.h>
#include <solid/deviceinterface.h>

// Local
#include "core/favoritesmodel.h"
#include "core/recentlyusedmodel.h"
#include "core/applicationmodel.h"
#include "core/leavemodel.h"
#include "core/itemhandlers.h"
#include "core/searchmodel.h"
#include "core/systemmodel.h"

#include "ui/itemdelegate.h"
#include "ui/contextmenufactory.h"
#include "ui/urlitemview.h"
#include "ui/flipscrollview.h"
#include "ui/searchbar.h"
#include "ui/tabbar.h"

using namespace Kickoff;

class Launcher::Private
{
public:
    Private(Launcher *launcher)
        : q(launcher)
        , urlLauncher(new UrlItemLauncher(launcher))
        , searchBar(0)
        , contentArea(0)
        , contentSwitcher(0)
        , searchView(0)
        , favoritesView(0)
        , contextMenuFactory(0)
        , autoHide(false)
    {
    }
    ~Private()
    {
    }

    void setupEventHandler(QAbstractItemView *view)
    {
        view->viewport()->installEventFilter(q);
        view->installEventFilter(q);
    }

    void addView(const QString& name, const QIcon& icon,
                 QAbstractItemModel *model = 0, QAbstractItemView *view = 0)
    {
        view->setFrameStyle(QFrame::NoFrame);
        // prevent the view from stealing focus from the search bar
        view->setFocusPolicy(Qt::NoFocus);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setDragEnabled(true);
        view->setAcceptDrops(true);
        view->setDropIndicatorShown(true);
        view->setDragDropMode(QAbstractItemView::InternalMove);
        view->setModel(model);
        //view->setCurrentIndex(QModelIndex());
        setupEventHandler(view);

        connect(view, SIGNAL(customContextMenuRequested(QPoint)), q, SLOT(showViewContextMenu(QPoint)));

        contentSwitcher->addTab(icon, name);
        contentArea->addWidget(view);
    }

    void initTabs()
    {
        // Favorites view
        setupFavoritesView();

        // All Programs view
        setupAllProgramsView();

        // My Computer view
        setupSystemView();

        // Recently Used view
        setupRecentView();

        // Leave view
        setupLeaveView();

        // Search Bar
        setupSearchView();
    }

    void setupLeaveView()
    {
        LeaveModel *model = new LeaveModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate;
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Leave"), KIcon("application-exit"), model, view);
    }

    void setupFavoritesView()
    {
        FavoritesModel *model = new FavoritesModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate;
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Favorites"), KIcon("bookmark"), model, view);

        connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), q, SLOT(focusFavoritesView()));

        favoritesView = view;
    }

    void setupAllProgramsView()
    {
        ApplicationModel *applicationModel = new ApplicationModel(q);
        applicationModel->setDuplicatePolicy(ApplicationModel::ShowLatestOnlyPolicy);

        QAbstractItemView *applicationView = new FlipScrollView;
        ItemDelegate *delegate = new ItemDelegate;
        applicationView->setItemDelegate(delegate);

        addView(i18n("Applications"), KIcon("applications-other"),
                    applicationModel, applicationView);
    }

    void setupRecentView()
    {
        RecentlyUsedModel *model = new RecentlyUsedModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate;
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Recently Used"), KIcon("view-calendar-week"), model, view);

        QAction *clearApplications = new QAction(KIcon("edit-clear-history"), i18n("Clear Recent Applications"), q);
        QAction *clearDocuments = new QAction(KIcon("edit-clear-history"), i18n("Clear Recent Documents"), q);

        connect(clearApplications, SIGNAL(triggered()), model, SLOT(clearRecentApplications()));
        connect(clearDocuments, SIGNAL(triggered()), model, SLOT(clearRecentDocuments()));

        contextMenuFactory->setViewActions(view, QList<QAction*>() << clearApplications << clearDocuments);
    }

    void setupSystemView()
    {
        SystemModel *model = new SystemModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate;
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);

        addView(i18n("My Computer"), systemIcon(), model, view);
    }

    void setupSearchView()
    {
        SearchModel *model = new SearchModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate;
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        view->setModel(model);
        view->setFrameStyle(QFrame::NoFrame);
        // prevent view from stealing focus from the search bar
        view->setFocusPolicy(Qt::NoFocus);
        setupEventHandler(view);

        connect(searchBar, SIGNAL(queryChanged(QString)), model, SLOT(setQuery(QString)));
        connect(searchBar, SIGNAL(queryChanged(QString)), q, SLOT(focusSearchView(QString)));

        view->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(view, SIGNAL(customContextMenuRequested(QPoint)), q, SLOT(showViewContextMenu(QPoint)));

        contentArea->addWidget(view);
        searchView = view;
    }

    void registerUrlHandlers()
    {
        UrlItemLauncher::addGlobalHandler(UrlItemLauncher::ExtensionHandler, "desktop", new ServiceItemHandler);
        UrlItemLauncher::addGlobalHandler(UrlItemLauncher::ProtocolHandler, "leave", new LeaveItemHandler);
    }

    QIcon systemIcon()
    {
       QList<Solid::Device> batteryList = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());

       if (batteryList.isEmpty()) {
          return KIcon("computer");
       } else {
          return KIcon("computer-laptop");
       }
    }

    Launcher * const q;
    UrlItemLauncher *urlLauncher;
    SearchBar *searchBar;
    QStackedWidget *contentArea;
    TabBar *contentSwitcher;
    QAbstractItemView *searchView;
    QAbstractItemView *favoritesView;
    ContextMenuFactory *contextMenuFactory;
    bool autoHide;
};

Launcher::Launcher(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , d(new Private(this))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    d->searchBar = new SearchBar(this);
    d->searchBar->installEventFilter(this);
    d->contentArea = new QStackedWidget(this);
    d->contentSwitcher = new TabBar(this);
    d->contentSwitcher->installEventFilter(this);
    d->contentSwitcher->setIconSize(QSize(48,48));
    d->contentSwitcher->setShape(QTabBar::RoundedSouth);
    connect(d->contentSwitcher, SIGNAL(currentChanged(int)),
            d->contentArea, SLOT(setCurrentIndex(int)) );
    d->contextMenuFactory = new ContextMenuFactory(this);

    d->initTabs();
    d->registerUrlHandlers();

    layout->addWidget(d->searchBar);
    layout->addWidget(d->contentArea);
    layout->addWidget(d->contentSwitcher);

    setLayout(layout);
}

QSize Launcher::sizeHint() const
{
    // TODO This is essentially an arbitrarily chosen height which works
    // reasonably well.  It would probably be better to choose the
    // height to allow X number of items to be visible in the Favorites
    // view (which shows initially on startup)
    QSize size = QWidget::sizeHint();

    // the extra 2 pixels are to make room for the content margins; see moveEvent
    size.rwidth() += 2;
    size.rheight() += 102;

    return size;
}

void Launcher::setAutoHide(bool hide)
{
    d->autoHide = hide;
}

bool Launcher::autoHide() const
{
    return d->autoHide;
}

void Launcher::setSwitchTabsOnHover(bool switchOnHover)
{
    d->contentSwitcher->setSwitchTabsOnHover(switchOnHover);
}

bool Launcher::switchTabsOnHover() const
{
    return d->contentSwitcher->switchTabsOnHover();
}

Launcher::~Launcher()
{
    delete d;
}

void Launcher::focusSearchView(const QString& query)
{
    if (!query.isEmpty()) {
        d->contentArea->setCurrentWidget(d->searchView);
    } else {
        focusFavoritesView();
    }
}

void Launcher::focusFavoritesView()
{
    d->contentSwitcher->setCurrentIndex(0);
    d->contentArea->setCurrentWidget(d->favoritesView);
}

bool Launcher::eventFilter(QObject *object, QEvent *event)
{
    // deliver unhandled key presses from the search bar
    // (mainly arrow keys, enter) to the active view
    if ((object == d->contentSwitcher || object == d->searchBar) && event->type() == QEvent::KeyPress) {
            // we want left/right to still nav the tabbar
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::NoModifier &&
            (keyEvent->key() == Qt::Key_Left ||
             keyEvent->key() == Qt::Key_Right)) {
            if (object == d->contentSwitcher) {
                return false;
            } else {
                QCoreApplication::sendEvent(d->contentSwitcher, event);
                return true;
            }
        }

        QAbstractItemView *activeView = qobject_cast<QAbstractItemView*>(d->contentArea->currentWidget());
        if (activeView) {
            QCoreApplication::sendEvent(activeView, event);
            return true;
        }
    }


    // the mouse events we are interested in are delivered to the viewport,
    // other events are delivered to the view itself.
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(object);
    if(!view) {
        view = qobject_cast<QAbstractItemView*>(object->parent());
    }

    if (view) {
        QModelIndex openIndex;
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = (QMouseEvent*)event;
            const QModelIndex index = view->indexAt(mouseEvent->pos());
            if (index.isValid() &&
                index.model()->hasChildren(index) == false &&
                mouseEvent->button() == Qt::LeftButton) {
                openIndex = index;
            }
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent*)event;
            const QModelIndex index = view->currentIndex();
            if (index.isValid() && index.model()->hasChildren(index) == false &&
                (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)) {
                openIndex = index;
            }
        }
        if (openIndex.isValid()) {
            d->urlLauncher->openItem(openIndex);
            // Clear the search bar when enter was pressed
            if (event->type() == QEvent::KeyPress) {
                d->searchBar->clear();
            }
            if (d->autoHide) {
                hide();
            }
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}

void Launcher::showViewContextMenu(const QPoint& pos)
{
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(sender());
    if (view) {
        d->contextMenuFactory->showContextMenu(view, d->contentArea->mapFromParent(pos));
    }
}

void Launcher::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
#if 0
    // allow tab switching by pressing the left or right arrow keys
    if (event->key() == Qt::Key_Left && d->contentSwitcher->currentIndex() > 0) {
        d->contentSwitcher->setCurrentIndex(d->contentSwitcher->currentIndex()-1);
    } else if (event->key() == Qt::Key_Right &&
        d->contentSwitcher->currentIndex() < d->contentSwitcher->count()-1) {
        d->contentSwitcher->setCurrentIndex(d->contentSwitcher->currentIndex()+1);
    }
#endif
}

void Launcher::moveEvent(QMoveEvent *e)
{
    // focus the search bar ready for typing
    int leftMargin = 1;
    int rightMargin = 1;
    int topMargin = 1;
    int bottomMargin = 1;

    QRect r(QApplication::desktop()->screenGeometry(this));
    QRect g(geometry());

    if (g.left() == r.left()) {
        leftMargin = 0;
    }

    if (g.right() == r.right()) {
        rightMargin = 0;
    }

    if (g.top() == r.top()) {
        topMargin = 0;
    }

    if (g.bottom() == r.bottom()) {
        bottomMargin = 0;
    }

    setContentsMargins(leftMargin, rightMargin, topMargin, rightMargin);
    QWidget::moveEvent(e);
}

void Launcher::showEvent(QShowEvent *e)
{
    d->searchBar->setFocus();
    QWidget::showEvent(e);
}

void Launcher::hideEvent(QHideEvent *e)
{
    emit aboutToHide();
    QWidget::hideEvent(e);
}

void Launcher::paintEvent(QPaintEvent*)
{
    // TODO - Draw a pretty background here
    QPainter p(this);
    p.setPen(QPen(palette().mid(), 0));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}
