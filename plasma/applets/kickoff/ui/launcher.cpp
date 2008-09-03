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

#include "ui/launcher.h"

// System
#include <unistd.h>

// Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStyleOptionSizeGrip>

// KDE
#include <KDebug>
#include <KLocalizedString>
#include <KIcon>
#include <KStandardDirs>
#include <ktoolinvocation.h>
#include <kuser.h>
#include <plasma/theme.h>
#include <plasma/delegate.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <KColorScheme>

// Local
#include "core/favoritesmodel.h"
#include "core/recentlyusedmodel.h"
#include "core/applicationmodel.h"
#include "core/leavemodel.h"
#include "core/itemhandlers.h"
#include "core/searchmodel.h"
#include "core/systemmodel.h"

#include "ui/itemdelegate.h"
#include "ui/brandingbutton.h"
#include "ui/contextmenufactory.h"
#include "ui/urlitemview.h"
#include "ui/flipscrollview.h"
#include "ui/panelsvgwidget.h"
#include "ui/searchbar.h"
#include "ui/tabbar.h"

using namespace Kickoff;

class Launcher::Private
{
public:
    enum CompassDirection { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };

    Private(Launcher *launcher)
        : q(launcher)
        , applet(0)
        , urlLauncher(new UrlItemLauncher(launcher))
        , searchModel(0)
        , searchBar(0)
        , footer(0)
        , contentArea(0)
        , contentAreaBorder(0)
        , contentSwitcher(0)
        , searchView(0)
        , favoritesView(0)
        , contextMenuFactory(0)
        , autoHide(false)
        , visibleItemCount(10)
        , launcherOrigin(QPoint(-1, -1))
        , isResizing(false)
        , resizePlacement( NorthEast )
        , panelEdge( Plasma::LeftEdge )
    {
    }

    ~Private()
    {
    }

    enum TabOrder { NormalTabOrder, ReverseTabOrder };

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

        // System view
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
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Leave"), KIcon("system-shutdown"), model, view);
    }

    void setupFavoritesView()
    {
        FavoritesModel *model = new FavoritesModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Favorites"), KIcon("bookmarks"), model, view);

        connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), q, SLOT(focusFavoritesView()));

        favoritesView = view;
    }

    void setupAllProgramsView()
    {
        ApplicationModel *applicationModel = new ApplicationModel(q);
        applicationModel->setDuplicatePolicy(ApplicationModel::ShowLatestOnlyPolicy);

        applicationView = new FlipScrollView();
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        applicationView->setItemDelegate(delegate);

        addView(i18n("Applications"), KIcon("applications-other"),
                    applicationModel, applicationView);
    }

    void setupRecentView()
    {
        RecentlyUsedModel *model = new RecentlyUsedModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        addView(i18n("Recently Used"), KIcon("document-open-recent"), model, view);

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
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);

        addView(i18n("Computer"), systemIcon(), model, view);
    }

    void setupSearchView()
    {
        searchModel = new SearchModel(q);
        UrlItemView *view = new UrlItemView;
        ItemDelegate *delegate = new ItemDelegate(q);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleRole, SubTitleRole);
        delegate->setRoleMapping(Plasma::Delegate::SubTitleMandatoryRole, SubTitleMandatoryRole);
        view->setItemDelegate(delegate);
        view->setItemStateProvider(delegate);
        view->setModel(searchModel);
        view->setFrameStyle(QFrame::NoFrame);
        // prevent view from stealing focus from the search bar
        view->setFocusPolicy(Qt::NoFocus);
        view->setDragEnabled(true);
        setupEventHandler(view);

        connect(searchModel, SIGNAL(resultsAvailable()), q, SLOT(resultsAvailable()));

        connect(searchBar, SIGNAL(queryChanged(QString)), searchModel, SLOT(setQuery(QString)));
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

    void setNorthLayout(TabOrder tabOrder)
    {
        contentSwitcher->setShape( QTabBar::RoundedNorth );
        QLayout * layout = q->layout();
        delete layout;
        layout = new QVBoxLayout();
        layout->addWidget(contentSwitcher);
        layout->addWidget(contentAreaBorder);
        layout->addWidget(searchBar);
        layout->addWidget(footer);
        layout->setSpacing(0);
        layout->setMargin(0);
        q->setLayout(layout);
        setTabOrder( tabOrder );
    }

    void setSouthLayout(TabOrder tabOrder)
    {
        contentSwitcher->setShape( QTabBar::RoundedSouth );
        QLayout * layout = q->layout();
        delete layout;
        layout = new QVBoxLayout();
        layout->addWidget(footer);
        layout->addWidget(searchBar);
        layout->addWidget(contentAreaBorder);
        layout->addWidget(contentSwitcher);
        layout->setSpacing(0);
        layout->setMargin(0);
        q->setLayout(layout);
        setTabOrder( tabOrder );
    }

    void setWestLayout(TabOrder tabOrder)
    {
        contentSwitcher->setShape( QTabBar::RoundedWest );
        QLayout * layout = q->layout();
        delete layout;
        layout = new QHBoxLayout();
        layout->addWidget(contentSwitcher);
        layout->addWidget(contentAreaBorder);
        QBoxLayout * layout2 = new QVBoxLayout();
        if ( tabOrder == NormalTabOrder ) {
            layout2->addLayout(layout);
            layout2->addWidget(searchBar);
            layout2->addWidget(footer);
        } else {
            layout2->addWidget(footer);
            layout2->addWidget(searchBar);
            layout2->addLayout(layout);
        }
        layout->setSpacing(0);
        layout->setMargin(0);
        layout2->setSpacing(0);
        layout2->setMargin(0);
        q->setLayout(layout2);
        setTabOrder( tabOrder );
    }

    void setEastLayout(TabOrder tabOrder)
    {
        contentSwitcher->setShape( QTabBar::RoundedEast );
        QLayout * layout = q->layout();
        delete layout;
        layout = new QHBoxLayout();
        layout->addWidget(contentAreaBorder);
        layout->addWidget(contentSwitcher);
        QBoxLayout * layout2 = new QVBoxLayout();
        if ( tabOrder == NormalTabOrder ) {
            layout2->addLayout(layout);
            layout2->addWidget(searchBar);
            layout2->addWidget(footer);
        } else {
            layout2->addWidget(footer);
            layout2->addWidget(searchBar);
            layout2->addLayout(layout);
        }
        layout->setSpacing(0);
        layout->setMargin(0);
        layout2->setSpacing(0);
        layout2->setMargin(0);
        q->setLayout(layout2);
        setTabOrder(tabOrder);
    }

    void setTabOrder(TabOrder newOrder)
    {
        // identify current TabOrder, assumes favoritesView is first in normal order
        TabOrder oldOrder;
        if ( contentArea->widget( 0 ) == favoritesView  ) {
            oldOrder = NormalTabOrder;
        } else {
            oldOrder = ReverseTabOrder;
        }
        if ( newOrder == oldOrder ) {
            return;
        }
        // remove the and widgets and store their data in a separate structure
        // remove this first so we can cleanly remove the widgets controlled by contentSwitcher
        contentArea->removeWidget( searchView );
        Q_ASSERT( contentArea->count() == contentSwitcher->count() );

        QList<WidgetTabData> removedTabs;
        for ( int i = contentSwitcher->count() - 1; i >= 0 ; i-- ) {
            WidgetTabData wtd;
            wtd.tabText = contentSwitcher->tabText( i );
            wtd.tabToolTip = contentSwitcher->tabToolTip( i );
            wtd.tabWhatsThis = contentSwitcher->tabWhatsThis( i );
            wtd.tabIcon = contentSwitcher->tabIcon( i );
            wtd.widget = contentArea->widget( i );
            removedTabs.append( wtd );

            contentSwitcher->removeTab( i );
            contentArea->removeWidget( contentArea->widget( i ) );
        }
        // then reinsert them in reversed order
        int i = 0;
        foreach (const WidgetTabData &wtd, removedTabs) {
            contentSwitcher->addTab( wtd.tabIcon, wtd.tabText );
            contentSwitcher->setTabToolTip( i, wtd.tabToolTip );
            contentSwitcher->setTabWhatsThis( i, wtd.tabWhatsThis );
            contentArea->addWidget( wtd.widget );
            ++i;
        }
        //finally replace the searchView
        contentArea->addWidget( searchView );
    }

    struct WidgetTabData
    {
        QString tabText;
        QString tabToolTip;
        QString tabWhatsThis;
        QIcon tabIcon;
        QWidget * widget;
    };

    Launcher * const q;
    Plasma::Applet *applet;
    UrlItemLauncher *urlLauncher;
    SearchModel *searchModel;
    SearchBar *searchBar;
    QWidget *footer;
    QStackedWidget *contentArea;
    PanelSvgWidget *contentAreaBorder;
    TabBar *contentSwitcher;
    FlipScrollView *applicationView;
    QAbstractItemView *searchView;
    QAbstractItemView *favoritesView;
    ContextMenuFactory *contextMenuFactory;
    bool autoHide;
    int visibleItemCount;
    QPoint launcherOrigin;
    bool isResizing;
    int resizeOffsetX;
    int resizeOffsetY;
    CompassDirection resizePlacement;
    Plasma::Location panelEdge;
};

Launcher::Launcher(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , d(new Private(this))
{
    init();
}

Launcher::Launcher(Plasma::Applet *applet)
    : QWidget(0, Qt::Window)
    , d(new Private(this))
{
    init();
    setApplet(applet);
}

void Launcher::init()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    const int rightHeaderMargin = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    d->searchBar = new SearchBar(this);
    if (layoutDirection() == Qt::LeftToRight) {
        d->searchBar->setContentsMargins(0, 0, rightHeaderMargin, 0);
    } else {
        d->searchBar->setContentsMargins(rightHeaderMargin, 0, 0, 0);
    }
    d->searchBar->installEventFilter(this);
    d->contentArea = new QStackedWidget(this);
    d->contentAreaBorder = new PanelSvgWidget(this);
    QVBoxLayout *contentAreaBorderLayout = new QVBoxLayout;
    contentAreaBorderLayout->addWidget(d->contentArea);
    d->contentAreaBorder->setLayout(contentAreaBorderLayout);

    d->contentSwitcher = new TabBar(this);
    d->contentSwitcher->installEventFilter(this);
    d->contentSwitcher->setIconSize(QSize(48,48));
    d->contentSwitcher->setShape(QTabBar::RoundedSouth);
    connect(d->contentSwitcher, SIGNAL(currentChanged(int)),
            d->contentArea, SLOT(setCurrentIndex(int)) );
    d->contextMenuFactory = new ContextMenuFactory(this);

    d->initTabs();
    d->registerUrlHandlers();

    // Add status information footer
    d->footer = new QWidget;

    char hostname[256];
    hostname[0] = '\0';
    if (!gethostname( hostname, sizeof(hostname) )) {
       hostname[sizeof(hostname)-1] = '\0';
    }
    KUser user;
    QString fullName = user.property(KUser::FullName).toString();
    QString labelText;
    if (fullName.isEmpty()) {
        labelText = i18nc("login name, hostname", "user&nbsp;<b>%1</b>&nbsp;on&nbsp;<b>%2</b>", user.loginName(), hostname);
    } else {
        labelText = i18nc("full name, login name, hostname", "<b>%1 (%2)</b>&nbsp;on&nbsp;<b>%3</b>", fullName, user.loginName(), hostname);
    }
    QLabel *userinfo = new QLabel(labelText);
    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette p = userinfo->palette();
    p.setColor(QPalette::Normal, QPalette::WindowText, color);
    p.setColor(QPalette::Inactive, QPalette::WindowText, color);
    userinfo->setPalette(p);

    QToolButton *branding = new BrandingButton(this);
    branding->setAutoRaise(false);
    branding->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect( branding, SIGNAL(clicked()), SLOT(openHomepage()));

    QHBoxLayout *brandingLayout = new QHBoxLayout;
    brandingLayout->setMargin(3);
    brandingLayout->addSpacing(ItemDelegate::ITEM_LEFT_MARGIN - 3);
    brandingLayout->addWidget(userinfo);
    brandingLayout->addStretch(2);
    brandingLayout->addWidget(branding);
    brandingLayout->addSpacing(rightHeaderMargin);
    d->footer->setLayout(brandingLayout);

    layout->addWidget(d->footer);
    layout->addWidget(d->searchBar);
    layout->addWidget(d->contentAreaBorder);
    layout->addWidget(d->contentSwitcher);

    setLayout(layout);
    //setBackgroundRole(QPalette::AlternateBase);
    //setAutoFillBackground(true);
    d->resizePlacement = Private::NorthEast;
}

QSize Launcher::minimumSizeHint() const
{
    QSize size = QWidget::sizeHint();

    // the extra 2 pixels are to make room for the content margins; see moveEvent
    const int CONTENT_MARGIN_WIDTH = 2;

    size.rwidth() += CONTENT_MARGIN_WIDTH;

    switch ( d->panelEdge ) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            size.rheight() = d->searchBar->sizeHint().height() +
                     d->footer->sizeHint().height() +
                     qMax( d->favoritesView->sizeHintForRow(0)*3 + ItemDelegate::HEADER_HEIGHT, d->contentSwitcher->sizeHint().height() );
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
        default:
            size.rheight() = d->searchBar->sizeHint().height() +
                     d->contentSwitcher->sizeHint().height() + d->footer->sizeHint().height() +
                     d->favoritesView->sizeHintForRow(0)*3 + ItemDelegate::HEADER_HEIGHT;
            break;
    }
    return size;
}

QSize Launcher::sizeHint() const
{
    KConfigGroup sizeGroup;
    if (d->applet) {
       sizeGroup = d->applet->config();
    } else {
       sizeGroup = componentData().config()->group("Size");
    }
    const int width = qMin(sizeGroup.readEntry("Width", 0), QApplication::desktop()->screen()->width()-50);
    const int height = qMin(sizeGroup.readEntry("Height", 0), QApplication::desktop()->screen()->height()-50);
    QSize wanted(width, height);
    bool isDefault = wanted.isNull();
    wanted = wanted.expandedTo(minimumSizeHint());
    if (isDefault) {
       wanted.setHeight( wanted.height() + ( d->favoritesView->sizeHintForRow(0) * (d->visibleItemCount - 3) ) );
    }

    return wanted;
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

void Launcher::setVisibleItemCount(int count)
{
    d->visibleItemCount = count;
}

int Launcher::visibleItemCount() const
{
    return d->visibleItemCount;
}

void Launcher::setApplet(Plasma::Applet *applet)
{
    d->applet = applet;
    d->contextMenuFactory->setApplet(applet);

    KConfigGroup cg = applet->globalConfig();
    setSwitchTabsOnHover(cg.readEntry("SwitchTabsOnHover", switchTabsOnHover()));

    cg = applet->config();
    setVisibleItemCount(cg.readEntry("VisibleItemsCount", visibleItemCount()));
}

void Launcher::reset()
{
    d->contentSwitcher->setCurrentIndexWithoutAnimation(d->contentArea->indexOf(d->favoritesView));
    d->contentArea->setCurrentWidget(d->favoritesView);
    d->searchBar->clear();
    d->applicationView->viewRoot();
}

Launcher::~Launcher()
{
    delete d;
}

void Launcher::focusSearchView(const QString& query)
{
    bool queryEmpty = query.isEmpty();

    d->contentSwitcher->setVisible(queryEmpty);

    if (!queryEmpty) {
        d->contentArea->setCurrentWidget(d->searchView);
    } else {
        focusFavoritesView();
    }
}

void Launcher::focusFavoritesView()
{
    d->contentSwitcher->setCurrentIndex(d->contentArea->indexOf(d->favoritesView));
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
                emit aboutToHide();
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
        d->contextMenuFactory->showContextMenu(view, pos);
    }
}

void Launcher::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    reset();
}

void Launcher::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit aboutToHide();
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

void Launcher::showEvent(QShowEvent *e)
{
    d->searchBar->setFocus();
    QWidget::showEvent(e);
}

void Launcher::openHomepage()
{
    emit aboutToHide();
    KToolInvocation::invokeBrowser("http://www.kde.org/");
}

void Launcher::resultsAvailable()
{
    const QModelIndex root = d->searchModel->index(0, 0);
    d->searchView->setCurrentIndex(d->searchModel->index(0, 0, root));
}

void Launcher::setLauncherOrigin(const QPoint &origin, Plasma::Location location)
{
/* 8 interesting positions for the menu to popup, depending where
 * the launcher and panel it is on are sited:
 *
 * K3PANELPANELPANEL4K
 * 2                 5
 * P                 P
 * A                 A
 * N                 N
 * E                 E
 * L                 L
 * 1                 6
 * K8PANELPANELPANEL7K
 *
 * Position determines optimum layout according to Fitt's Law:
 * Assumption 1: The hardcoded tab order defines a desirable priority order
 * Goal 1: TabBar perpendicular to direction of mouse travel from launcher to target, to prevent
 * mousing over non-target tabs and potential unnecessary tab switches
 * Goal 2 the movie: Tabs are ordered by decreasing priority along the mouse travel vector away
 * from the launcher
 * Constraint: The search widget is different to the tabs and footer is of lowest priority so 
 * these should always be situated furthest away from the origin
 *
 * | Position | TabLayout | TabOrder   | rx | ry
 * |    1     |  South    | left2right | =  | -
 * |    2     |  North    | l2r        | =  | +
 * |    3     |  West     | top2bottom | +  | =
 * |    4     |  East     | t2b        | -  | =
 * |    5     |  North    | r2l        | -  | +
 * |    6     |  South    | r2l        | -  | -
 * |    7     |  East     | b2t        | -  | -
 * |    8     |  West     | b2t        | +  | +
 */
    if (d->launcherOrigin == origin && d->panelEdge == location) {
        return;
    }

    //should never happen, or happen if this function is called when the menu is in
    //a planar containment, where this function is useless
    if (!parentWidget()) {
        return;
    }

    d->launcherOrigin = origin;
    d->panelEdge = location;
    QPoint relativePosition = parentWidget()->pos() - d->launcherOrigin;
    int rx = relativePosition.x();
    int ry = relativePosition.y();

    if ( rx < 0 ) {
        if ( ry < 0 ) {
            if ( location == Plasma::RightEdge ) {
                // Position 7
                kDebug() << "menu position " << 7;
                d->resizePlacement = Private::NorthWest;
                d->setEastLayout( Private::ReverseTabOrder );
            } else { // Plasma::BottomEdge
                // Position 6
                kDebug() << "menu position " << 6;
                d->resizePlacement = Private::NorthWest;
                d->setSouthLayout( Private::ReverseTabOrder );
            }
        } else if ( ry == 0 ) {
            // Position 4
            kDebug() << "menu position " << 4;
            d->resizePlacement = Private::SouthWest;
            d->setEastLayout( Private::NormalTabOrder );
        } else {
            // Position 5
            kDebug() << "menu position " << 5;
            d->resizePlacement = Private::SouthWest;
            d->setNorthLayout( Private::ReverseTabOrder );
        }
    } else if ( rx == 0 ) {
        if ( ry < 0 ) {
            // Position 1
            kDebug() << "menu position " << 1;
            d->resizePlacement = Private::NorthEast;
            d->setSouthLayout( Private::NormalTabOrder );
        } else {
            // Position 2
            kDebug() << "menu position " << 2;
            d->resizePlacement = Private::SouthEast;
            d->setNorthLayout( Private::NormalTabOrder );
        }
    } else { // rx > 0
        if ( ry == 0 ) {
            // Position 3
            kDebug() << "menu position " << 3;
            d->resizePlacement = Private::SouthEast;
            d->setWestLayout( Private::NormalTabOrder );
        } else { //ry > 0
            // Position 8
            kDebug() << "menu position " << 8;
            d->resizePlacement = Private::NorthEast;
            d->setWestLayout( Private::ReverseTabOrder );
        }
    }
}

QPoint Launcher::launcherOrigin() const
{
    return d->launcherOrigin;
}

void Launcher::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->isResizing) {
       d->isResizing = false;
       KConfigGroup sizeGroup;
       if (d->applet) {
          sizeGroup = d->applet->config();
       } else {
          sizeGroup = componentData().config()->group("Size");
       }
       sizeGroup.writeEntry("Height", height());
       sizeGroup.writeEntry("Width", width());
       emit configNeedsSaving();
    }
    QWidget::mouseReleaseEvent(e);
}

#include "launcher.moc"

