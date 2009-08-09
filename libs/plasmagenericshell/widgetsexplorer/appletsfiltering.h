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

        enum FilteringListOrientation {
            Vertical = 0,
            Horizontal = 1
        };

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

class FilteringWidgetWithTabs : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit FilteringWidgetWithTabs(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~FilteringWidgetWithTabs();

        void init();
        FilteringTabs *categoriesList();
        Plasma::LineEdit *textSearch();

    private:
        FilteringTabs *m_categoriesList;
        Plasma::LineEdit *m_textSearch;
        Plasma::Label *m_filterLabel;
};

#endif // APPLETSFILTERING_H
