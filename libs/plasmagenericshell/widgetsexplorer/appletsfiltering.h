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

class FilteringList : public QGraphicsWidget
{

    Q_OBJECT

    public:
        explicit FilteringList(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~FilteringList();

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
        void wheelEvent(QGraphicsSceneWheelEvent *event);

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
        virtual ~FilteringWidget();

        void init();
        FilteringList *categoriesList();
        Plasma::LineEdit *textSearch();

        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        Plasma::FrameSvg *m_backgroundSvg;

        FilteringList *m_categoriesList;
        Plasma::LineEdit *m_textSearch;
        Plasma::Label *m_filterLabel;

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
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        Plasma::FrameSvg *m_backgroundSvg;

        FilteringTabs *m_categoriesList;
        Plasma::LineEdit *m_textSearch;
        Plasma::Label *m_filterLabel;
};

#endif // APPLETSFILTERING_H
