/***************************************************************************
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
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

#ifndef WEBBROWSER_H
#define WEBBROWSER_H

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

#include "ui_webbrowserconfig.h"

class QGraphicsLinearLayout;
class QStandardItemModel;
class QStandardItem;
class QTimer;
class KUrlPixmapProvider;
class KUrl;
class KCompletion;
class KBookmarkManager;
class KBookmarkGroup;
class QModelIndex;
class QAction;
class BookmarksDelegate;
class BookmarkItem;

namespace Plasma
{
    class IconWidget;
    class Meter;
    class HistoryComboBox;
    class WebView;
    class TreeView;
    class Slider;
}

class WebBrowser : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    WebBrowser(QObject *parent, const QVariantList &args);
    ~WebBrowser();

    QGraphicsWidget *graphicsWidget();

    //TODO: put in a separate file
    enum BookmarkRoles
    {
        UrlRole = Qt::UserRole+1,
        BookmarkRole = Qt::UserRole+2
    };

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected:
    void saveState(KConfigGroup &cg) const;
    Plasma::IconWidget *addTool(const QString &iconString, QGraphicsLinearLayout *layout);
    void createConfigurationInterface(KConfigDialog *parent);

protected Q_SLOTS:
    void back();
    void forward();
    void reload();
    void returnPressed();
    void urlChanged(const QUrl &url);
    void comboTextChanged(const QString &string);
    void addBookmark();
    void removeBookmark(const QModelIndex &index);
    void removeBookmark();
    void bookmarksToggle();
    void bookmarkClicked(const QModelIndex &index);
    void zoom(int value);
    void loadProgress(int progress);
    void bookmarksModelInit();
    void configAccepted();

private:
    void fillGroup(BookmarkItem *parentItem, const KBookmarkGroup &group);

    QGraphicsLinearLayout *m_layout;
    QGraphicsLinearLayout *m_toolbarLayout;
    QGraphicsLinearLayout *m_statusbarLayout;
    Plasma::WebView *m_browser;
    KUrl m_url;
    int m_verticalScrollValue;
    int m_horizontalScrollValue;
    KUrlPixmapProvider *m_pixmapProvider;
    KCompletion *m_completion;
    KBookmarkManager *m_bookmarkManager;
    QStandardItemModel *m_bookmarkModel;
    Plasma::TreeView *m_bookmarksView;

    QTimer *m_autoRefreshTimer;
    bool m_autoRefresh;
    int m_autoRefreshInterval;

    QGraphicsWidget *m_graphicsWidget;

    Plasma::HistoryComboBox *m_historyCombo;
    BookmarksDelegate *m_bookmarksDelegate;

    Plasma::IconWidget *m_back;
    Plasma::IconWidget *m_forward;

    Plasma::IconWidget *m_go;
    QAction *m_goAction;
    QAction *m_reloadAction;

    Plasma::IconWidget *m_addBookmark;
    QAction *m_addBookmarkAction;
    QAction *m_removeBookmarkAction;

    Plasma::IconWidget *m_organizeBookmarks;
    Plasma::IconWidget *m_stop;
    Plasma::Meter *m_progress;
    Plasma::Slider *m_zoom;

    Ui::WebBrowserConfig ui;
};

K_EXPORT_PLASMA_APPLET(webbrowser, WebBrowser)

#endif
