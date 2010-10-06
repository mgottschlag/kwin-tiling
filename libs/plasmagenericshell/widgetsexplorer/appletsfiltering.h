/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLETSFILTERING_H
#define APPLETSFILTERING_H

#include <Plasma/PushButton>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"

class KMenu;
namespace Plasma {
    class LineEdit;
    class PushButton;
    class ToolButton;
    class WidgetExplorer;
}

class CategoriesWidget : public Plasma::PushButton
{
    Q_OBJECT

public:
    explicit CategoriesWidget(QGraphicsWidget *parent = 0);
    virtual ~CategoriesWidget();

    void setModel(QStandardItemModel *model);

Q_SIGNALS:
    void filterChanged(int index);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    //uses model to populate the tabs
    void populateList();
    QStandardItem *getItemByProxyIndex(const QModelIndex &index) const;

private Q_SLOTS:
    void menuItemTriggered(QAction *);
    void unpressButton();

private:
    QStandardItemModel *m_model;
    QMenu *m_menu;
};

class FilteringWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit FilteringWidget(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
    explicit FilteringWidget(Qt::Orientation orientation = Qt::Horizontal,
                             Plasma::WidgetExplorer* widgetExplorer = 0,
                             QGraphicsItem * parent = 0,
                             Qt::WindowFlags wFlags = 0);
    virtual ~FilteringWidget();

    void setModel(QStandardItemModel *model);
    void setListOrientation(Qt::Orientation orientation);
    Plasma::LineEdit *textSearch();
    void updateActions(const QList<QAction *> actions);

Q_SIGNALS:
    void filterChanged(int index);
    void closeClicked();

protected Q_SLOTS:
    void setMenuPos();
    void populateWidgetsMenu();

    /**
     * Launches a download dialog to retrieve new applets from the Internet
     *
     * @arg type the type of widget to download; an empty string means the default
     *           Plasma widgets will be accessed, any other value should map to a
     *           PackageStructure PluginInfo-Name entry that provides a widget browser.
     */
    void downloadWidgets(const QString &type = QString());

    /**
     * Opens a file dialog to open a widget from a local file
     */
    void openWidgetFile();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:
    void init();

    QStandardItemModel *m_model;
    CategoriesWidget *m_categories;
    Plasma::LineEdit *m_textSearch;
    Qt::Orientation m_orientation;
    Plasma::PushButton *m_newWidgetsButton;
    KMenu *m_newWidgetsMenu;
    Plasma::WidgetExplorer *m_widgetExplorer;
    QList<QWeakPointer<Plasma::PushButton> > m_actionButtons;
    Plasma::ToolButton *m_closeButton;
};

#endif // APPLETSFILTERING_H
