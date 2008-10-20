/***************************************************************************
 *   Copyright (C) 2007-2008 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "webbrowser.h"

#include <limits.h>

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QAction>
#include <QTimer>
#include <QTreeView>

#include <KIcon>
#include <KCompletion>
#include <KBookmarkManager>
#include <KIconLoader>
#include <KUrlPixmapProvider>
#include <KUriFilter>
#include <KMessageBox>
#include <KConfigDialog>
#include <KHistoryComboBox>

#include <plasma/widgets/icon.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/meter.h>
#include <plasma/widgets/webcontent.h>
#include <plasma/widgets/treeview.h>
#include <plasma/widgets/slider.h>

#include "historycombobox.h"
#include "bookmarksdelegate.h"
#include "bookmarkitem.h"

WebBrowser::WebBrowser(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
          m_verticalScrollValue(0),
          m_horizontalScrollValue(0),
          m_bookmarkModel(0),
          m_autoRefreshTimer(0)

{
    setHasConfigurationInterface(true);

    m_layout = 0;
    resize(500,500);
    if (args.count() > 0) {
        m_url = KUrl(args.value(0).toString());
    }
}

void WebBrowser::init()
{
    KConfigGroup cg = config();


    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_toolbarLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_statusbarLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    m_back = addTool("go-previous", m_toolbarLayout);
    m_forward = addTool("go-next", m_toolbarLayout);

    m_historyCombo = new Plasma::HistoryComboBox(this);
    m_historyCombo->setZValue(999);
    m_historyCombo->setDuplicatesEnabled(false);
    m_pixmapProvider = new KUrlPixmapProvider;
    m_historyCombo->nativeWidget()->setPixmapProvider(m_pixmapProvider);

    m_toolbarLayout->addItem(m_historyCombo);
    m_go = addTool("go-jump-locationbar", m_toolbarLayout);
    m_goAction = m_go->action();
    m_reloadAction = new QAction(KIcon("view-refresh"), QString(), this);

    m_layout->addItem(m_toolbarLayout);


    m_browser = new Plasma::WebContent(this);
    //FIXME: sounds wrong, but...
    m_browser->setPreferredSize(INT_MAX, INT_MAX);


    m_layout->addItem(m_browser);

    //bookmarks
    m_bookmarkManager = KBookmarkManager::userBookmarksManager();
    connect(m_bookmarkManager, SIGNAL(changed(const QString, const QString)), this, SLOT(bookmarksModelInit()));
    bookmarksModelInit();

    m_bookmarksView = new Plasma::TreeView(this);
    m_bookmarksView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bookmarksView->setModel(m_bookmarkModel);
    m_bookmarksView->nativeWidget()->setHeaderHidden(true);
    //m_bookmarksView->nativeWidget()->viewport()->setAutoFillBackground(false);
    m_bookmarksView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bookmarksView->setPreferredSize(INT_MAX, INT_MAX);
    m_bookmarksView->hide();
    //FIXME: this is probably a Qt bug
    m_bookmarksView->setMaximumHeight(0);
    m_layout->addItem(m_bookmarksView);

    m_bookmarksDelegate = new BookmarksDelegate(this);
    m_bookmarksView->nativeWidget()->setItemDelegate(m_bookmarksDelegate);

    connect(m_bookmarksDelegate, SIGNAL(destroyBookmark(const QModelIndex &)), this, SLOT(removeBookmark(const QModelIndex &)));

    m_layout->addItem(m_statusbarLayout);

    m_addBookmark = addTool("bookmark-new", m_statusbarLayout);
    m_addBookmarkAction = m_addBookmark->action();
    m_removeBookmarkAction = new QAction(KIcon("list-remove"), QString(), this);
    m_organizeBookmarks = addTool("bookmarks-organize", m_statusbarLayout);


    m_progress = new Plasma::Meter(this);
    m_progress->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);
    m_statusbarLayout->addItem(m_progress);
    m_stop = addTool("process-stop", m_statusbarLayout);

    setLayout(m_layout);

    m_zoom = new Plasma::Slider(this);
    m_zoom->setMaximum(100);
    m_zoom->setMinimum(0);
    m_zoom->setValue(50);
    m_zoom->setOrientation(Qt::Horizontal);
    m_zoom->hide();
    m_zoom->setMaximumWidth(0);
    m_statusbarLayout->addItem(m_zoom);
    connect(m_zoom, SIGNAL(sliderMoved(int)), this, SLOT(zoom(int)));

    if (!m_url.isValid()) {
        m_url = KUrl(cg.readEntry("Url", "http://www.kde.org"));
        m_verticalScrollValue = cg.readEntry("VerticalScrollValue", 0);
        m_horizontalScrollValue = cg.readEntry("HorizontalScrollValue", 0);
    }
    m_browser->setUrl(m_url);
    m_browser->update();

    m_autoRefresh = cg.readEntry("autoRefresh", false);
    m_autoRefreshInterval = qMax(2, cg.readEntry("autoRefreshInterval", 5));

    if (m_autoRefresh) {
        m_autoRefreshTimer = new QTimer(this);
        m_autoRefreshTimer->start(m_autoRefreshInterval*60*1000);
        connect(m_autoRefreshTimer, SIGNAL(timeout()), this, SLOT(reload()));
    }

    connect(m_back->action(), SIGNAL(triggered()), this, SLOT(back()));
    connect(m_forward->action(), SIGNAL(triggered()), this, SLOT(forward()));
    connect(m_reloadAction, SIGNAL(triggered()), this, SLOT(reload()));
    connect(m_goAction, SIGNAL(triggered()), this, SLOT(returnPressed()));
    connect(m_stop->action(), SIGNAL(triggered()), m_browser->page()->action(QWebPage::Stop), SLOT(trigger()));

    connect(m_historyCombo, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(m_historyCombo, SIGNAL(activated(int)), this, SLOT(returnPressed()));
    connect(m_historyCombo, SIGNAL(textChanged(const QString&)), this, SLOT(comboTextChanged(const QString&)));
    connect(m_browser->page()->mainFrame(), SIGNAL(urlChanged(const QUrl &)), this, SLOT(urlChanged(const QUrl &)));
    connect(m_browser, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));

    connect(m_addBookmarkAction, SIGNAL(triggered()), this, SLOT(addBookmark()));
    connect(m_removeBookmarkAction, SIGNAL(triggered()), this, SLOT(removeBookmark()));
    connect(m_organizeBookmarks->action(), SIGNAL(triggered()), this, SLOT(bookmarksToggle()));
    connect(m_bookmarksView->nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(bookmarkClicked(const QModelIndex &)));

    //Autocompletion stuff
    m_completion = new KCompletion();
    m_historyCombo->nativeWidget()->setCompletionObject(m_completion);

    QStringList list = cg.readEntry("History list", QStringList());
    m_historyCombo->setHistoryItems(list);

}

WebBrowser::~WebBrowser()
{
    KConfigGroup cg = config();
    saveState(cg);

    delete m_completion;
    delete m_bookmarkManager;
    delete m_bookmarkModel;
}

Plasma::Icon *WebBrowser::addTool(const QString &iconString, QGraphicsLinearLayout *layout)
{
    Plasma::Icon *icon = new Plasma::Icon(this);
    QAction *action = new QAction(KIcon(iconString), QString(), this);
    icon->setAction(action);
    icon->setPreferredSize(icon->sizeFromIconSize(IconSize(KIconLoader::Toolbar)));
    layout->addItem(icon);

    return icon;
}

void WebBrowser::bookmarksModelInit()
{
    if (m_bookmarkModel) {
        m_bookmarkModel->clear();
    } else {
        m_bookmarkModel = new QStandardItemModel;
    }

    fillGroup(0, m_bookmarkManager->root());
}

void WebBrowser::fillGroup(BookmarkItem *parentItem, const KBookmarkGroup &group)
{
    KBookmark it = group.first();

    while (!it.isNull()) {
        BookmarkItem *bookmarkItem = new BookmarkItem(it);
        bookmarkItem->setEditable(false);

        if (it.isGroup()) {
            KBookmarkGroup grp = it.toGroup();
            fillGroup( bookmarkItem, grp );

        }

        if (parentItem) {
            parentItem->appendRow(bookmarkItem);
        } else {
            m_bookmarkModel->appendRow(bookmarkItem);
        }

        it = m_bookmarkManager->root().next(it);
    }
}

void WebBrowser::saveState(KConfigGroup &cg) const
{
    cg.writeEntry("Url", m_url.prettyUrl());

    QStringList list = m_historyCombo->historyItems();
    cg.writeEntry("History list", list);

    cg.writeEntry("VerticalScrollValue", m_browser->page()->mainFrame()->scrollBarValue(Qt::Vertical));
    cg.writeEntry("HorizontalScrollValue", m_browser->page()->mainFrame()->scrollBarValue(Qt::Horizontal));
}

void WebBrowser::back()
{
    m_browser->page()->history()->back();
}

void WebBrowser::forward()
{
    m_browser->page()->history()->forward();
}

void WebBrowser::reload()
{
    m_browser->setUrl(m_url);
}

void WebBrowser::returnPressed()
{
    KUrl url(m_historyCombo->currentText());


    KUriFilter::self()->filterUri( url );

    m_verticalScrollValue = 0;
    m_horizontalScrollValue = 0;
    m_browser->setUrl(url);
}


void WebBrowser::urlChanged(const QUrl &url)
{
    //ask for a favicon
    Plasma::DataEngine *engine = dataEngine( "favicons" );
    if (engine) {
        //engine->disconnectSource( url.toString(), this );
        engine->connectSource( url.toString(), this );

        engine->query( url.toString() );
    }

    m_url = KUrl(url);

    if (m_bookmarkModel->match(m_bookmarkModel->index(0,0), BookmarkItem::UrlRole, m_url.prettyUrl()).isEmpty()) {
        m_addBookmark->setAction(m_addBookmarkAction);
    } else {
        m_addBookmark->setAction(m_removeBookmarkAction);
    }

    m_historyCombo->addToHistory(m_url.prettyUrl());
    m_historyCombo->nativeWidget()->setCurrentIndex(0);

    m_go->setAction(m_reloadAction);

    KConfigGroup cg = config();
    saveState(cg);

    m_back->setEnabled(m_browser->page()->history()->canGoBack());
    m_forward->setEnabled(m_browser->page()->history()->canGoForward());    
}

void WebBrowser::comboTextChanged(const QString &string)
{
    Q_UNUSED(string)

    m_go->setAction(m_goAction);
}

void WebBrowser::dataUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    //TODO: find a way to update bookmarks and history combobox here, at the moment the data engine
    // is only used to save the icon files
    if (source == m_historyCombo->currentText()) {
        QPixmap favicon(QPixmap::fromImage(data["Icon"].value<QImage>()));
        m_historyCombo->nativeWidget()->setItemIcon(m_historyCombo->nativeWidget()->currentIndex(), QIcon(favicon));
    }
}

void WebBrowser::addBookmark()
{
    KBookmark bookmark = m_bookmarkManager->root().addBookmark(m_browser->page()->mainFrame()->title(), m_url);
    m_bookmarkManager->save();

    BookmarkItem *bookmarkItem = new BookmarkItem(bookmark);
    m_bookmarkModel->appendRow(bookmarkItem);

    m_addBookmark->setAction(m_removeBookmarkAction);
}

void WebBrowser::removeBookmark(const QModelIndex &index)
{
    BookmarkItem *item = dynamic_cast<BookmarkItem *>(m_bookmarkModel->itemFromIndex(index));

    if (item) {
        KBookmark bookmark = item->bookmark();

        const QString text(i18nc("@info", "Do you really want to remove the bookmark to %1?", bookmark.url().host()));
        const bool del = KMessageBox::warningContinueCancel(0,
                                                            text,
                                                            QString(),
                                                            KGuiItem(i18nc("@action:button", "Delete bookmark"))
                                                            ) == KMessageBox::Continue;

        if (!del) {
            return;
        }

        bookmark.parentGroup().deleteBookmark(bookmark);
        m_bookmarkManager->save();
    }

    if (item && item->parent()) {
        item->parent()->removeRow(index.row());
    } else {
        m_bookmarkModel->removeRow(index.row());
    }

}

void WebBrowser::removeBookmark()
{
    QModelIndexList list = m_bookmarkModel->match(m_bookmarkModel->index(0,0), BookmarkItem::UrlRole, m_url.prettyUrl());

    if (!list.isEmpty()) {
        removeBookmark(list.first());
    }

    m_addBookmark->setAction(m_addBookmarkAction);
}

void WebBrowser::bookmarksToggle()
{
    if (m_bookmarksView->isVisible()) {
        m_bookmarksView->hide();
        m_bookmarksView->setMaximumHeight(0);
        m_browser->show();
        m_browser->setMaximumHeight(INT_MAX);
    } else {
        m_bookmarksView->show();
        m_bookmarksView->setMaximumHeight(INT_MAX);
        m_browser->hide();
        m_browser->setMaximumHeight(0);
    }
}

void WebBrowser::bookmarkClicked(const QModelIndex &index)
{
    QStandardItem *item = m_bookmarkModel->itemFromIndex(index);

    if (item) {
        KUrl url(item->data(BookmarkItem::UrlRole).value<QString>());

        if (url.isValid()) {
            m_browser->setUrl(url);
            bookmarksToggle();
        }
    }
}

void WebBrowser::zoom(int value)
{
    m_browser->mainFrame()->setTextSizeMultiplier((qreal)0.2 + ((qreal)value/(qreal)50));
}

void WebBrowser::loadProgress(int progress)
{
    m_progress->setValue(progress);
    m_progress->update();

    if (progress == 100) {
        m_progress->setMaximumWidth(0);
        m_progress->hide();
        m_stop->hide();
        m_stop->setMaximumWidth(0);
        m_zoom->show();
        m_zoom->setMaximumWidth(INT_MAX);
        m_statusbarLayout->invalidate();

        m_browser->page()->mainFrame()->setScrollBarValue(Qt::Vertical, m_verticalScrollValue);
        m_browser->page()->mainFrame()->setScrollBarValue(Qt::Horizontal, m_horizontalScrollValue);
    } else {
        m_progress->show();
        m_stop->show();
        m_stop->setMaximumWidth(INT_MAX);
        m_progress->setMaximumWidth(INT_MAX);
        m_zoom->setMaximumWidth(0);
        m_zoom->hide();
        m_statusbarLayout->invalidate();
    }
}

void WebBrowser::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.autoRefresh->setChecked(m_autoRefresh);
    ui.autoRefreshInterval->setValue(m_autoRefreshInterval);
}

void WebBrowser::configAccepted()
{
    KConfigGroup cg = config();

    m_autoRefresh = ui.autoRefresh->isChecked();
    m_autoRefreshInterval = ui.autoRefreshInterval->value();

    cg.writeEntry("autoRefresh", m_autoRefresh);
    cg.writeEntry("autoRefreshInterval", m_autoRefreshInterval);

    if (m_autoRefresh) {
        if (!m_autoRefreshTimer) {
            m_autoRefreshTimer = new QTimer(this);
            connect(m_autoRefreshTimer, SIGNAL(timeout()), this, SLOT(reload()));
        }

        m_autoRefreshTimer->start(m_autoRefreshInterval*60*1000);
    } else {
        delete m_autoRefreshTimer;
        m_autoRefreshTimer = 0;
    }

    emit configNeedsSaving();
}

#include "webbrowser.moc"
