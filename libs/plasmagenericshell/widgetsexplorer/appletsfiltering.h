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

#include <QtCore>
#include <QtGui>

#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/treeview.h>
#include <plasma/widgets/tabbar.h>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "widgetexplorer.h"

class FilteringTreeView : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit FilteringTreeView(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~FilteringTreeView();

        void init();
        void setModel(QStandardItemModel *model);

    private slots:
        void filterChanged(const QModelIndex &index);

    Q_SIGNALS:
        void filterChanged(int index);

    private:
        QStandardItemModel *m_model;
        Plasma::TreeView *m_treeView;
};

class FilteringTabs : public Plasma::TabBar
{
    Q_OBJECT

    public:
        explicit FilteringTabs(QGraphicsWidget *parent = 0);
        virtual ~FilteringTabs();

        void init();
        void setModel(QStandardItemModel *model);

    private:
        //uses model to populate the tabs
        void populateList();
        QStandardItem *getItemByProxyIndex(const QModelIndex &index) const;

    Q_SIGNALS:
        void filterChanged(int index);

    private:
        QStandardItemModel *m_model;

};

class FilteringWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit FilteringWidget(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        explicit FilteringWidget(Qt::Orientation orientation = Qt::Horizontal, QGraphicsItem * parent = 0,
                                 Qt::WindowFlags wFlags = 0);
        virtual ~FilteringWidget();

        void init();
        void setModel(QStandardItemModel *model);
        void setListOrientation(Qt::Orientation orientation);
        Plasma::LineEdit *textSearch();
        void resizeEvent(QGraphicsSceneResizeEvent *event);

    Q_SIGNALS:
        void filterChanged(int index);

    private:
        QGraphicsLinearLayout *m_linearLayout;
        FilteringTreeView *m_categoriesTreeView;
        FilteringTabs *m_categoriesTabs;
        Plasma::LineEdit *m_textSearch;
        Qt::Orientation m_orientation;
};

#endif // APPLETSFILTERING_H
