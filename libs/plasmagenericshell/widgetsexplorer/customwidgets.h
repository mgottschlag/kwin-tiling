#ifndef CUSTOMLAYOUTITEMS_H
#define CUSTOMLAYOUTITEMS_H

#include <QtGui>
#include <QtCore>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/pushbutton.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/treeview.h>
#include <KMenu>
#include <kpushbutton.h>
#include "plasmaappletitemmodel_p.h"
#include "kcategorizeditemsviewmodels_p.h"
#include  "klineedit.h"
#include <QScrollArea>
#include <plasma/framesvg.h>

class AppletIconWidget;
class PositionDotsSvgWidget;
class AppletsList;

class AppletsList : public QGraphicsWidget
{

    Q_OBJECT

public:
    AppletsList(QGraphicsItem *parent = 0);
    ~AppletsList();

    void init();

    KCategorizedItemsViewModels::AbstractItem *getItemByProxyIndex(const QModelIndex &index) const;
    void setItemModel(QStandardItemModel *model);
    void setFilterModel(QStandardItemModel *model);

    void populateAllAppletsHash();

    /**
     * Creates a new applet icon and puts it into the hash
     */
    AppletIconWidget *createAppletIcon(PlasmaAppletItem *appletItem);
    void insertAppletIcon(AppletIconWidget *appletIconWidget);
    void eraseList();

    QList < KCategorizedItemsViewModels::AbstractItem * > selectedItems() const;

public slots:
    void searchTermChanged(const QString &text);
    void filterChanged(int index);
    void updateList();
    void appletIconEnter(AppletIconWidget *appletIcon);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private slots:
//    void itemActivated(const QModelIndex &index);
//    void itemDoubleClicked(const QModelIndex &index);
//    void slotSearchTermChanged(const QString &term);


//    void itemClicked(const QModelIndex &index);
//    void itemEntered(const QModelIndex &index);

    //slot to handle the appletIcon and emit SIGNAL appletIconHoverEnter
//    void appletIconHoverEnter(AppletIconWidget *appletIcon);

Q_SIGNALS:

    void appletIconHoverEnter(PlasmaAppletItem *item);
    void appletIconHoverLeave(PlasmaAppletItem *item);

    void appletIconEnter(PlasmaAppletItem *appletItem);


private:
    /**
     * Hash containing all widgets that represents the applets
     */
    QHash<QString, AppletIconWidget *> *m_allAppletsHash;

    QGraphicsLinearLayout *m_appletListLinearLayout;
    QGraphicsWidget *m_appletsListWidget;
    QGraphicsWidget *m_appletsListWindowWidget;

    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::PushButton *m_rightArrow;
    Plasma::PushButton *m_leftArrow;

    QStandardItemModel *m_modelItems;
    QStandardItemModel *m_modelFilters;
    KCategorizedItemsViewModels::DefaultItemFilterProxyModel *m_modelFilterItems;

};

class AppletIconWidget : public Plasma::IconWidget
{

    Q_OBJECT

    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0, bool dotsSurrounded = true);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        PlasmaAppletItem *appletItem();

    public Q_SLOTS:
        void updateApplet(PlasmaAppletItem *newAppletItem);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);

    private:
        PlasmaAppletItem *m_appletItem;

};

class FilteringList : public QGraphicsWidget
{

    Q_OBJECT

    public:
        explicit FilteringList(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~FilteringList();

        void init();
        void setModel(QStandardItemModel *model);

    private:
        QStandardItemModel *m_model;
        Plasma::TreeView *m_treeView;

    Q_SIGNALS:
        void filterChanged(int index);

    public slots:
        void filterChanged(const QModelIndex &index);
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

class ManageWidgetsPushButton : public QGraphicsWidget
{

    Q_OBJECT

    public:
        explicit ManageWidgetsPushButton(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~ManageWidgetsPushButton();

        void init();
        KPushButton *button();
        QGraphicsProxyWidget *buttonProxy();
        KMenu *buttonMenu();

    private:
        KPushButton *m_button;
        QGraphicsProxyWidget *m_buttonProxy;
        KMenu *m_buttonMenu;
};


#endif // CUSTOMLAYOUTITEMS_H
