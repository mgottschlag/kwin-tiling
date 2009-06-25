#ifndef CUSTOMLAYOUTITEMS_H
#define CUSTOMLAYOUTITEMS_H

#include <QtGui>
#include <QtCore>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/pushbutton.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/treeview.h>
#include <KMenu>
#include <kpushbutton.h>
#include "standardcustomwidget.h"
#include "plasmaappletitemmodel_p.h"
#include "kcategorizeditemsview_p.h"
#include  "klineedit.h"

class AppletIconWidget;
class PositionDotsSvgWidget;
class AppletsList;

class AppletsListSearch : public StandardCustomWidget {

    Q_OBJECT;

public:
    explicit AppletsListSearch(QGraphicsItem *parent = 0);
    virtual ~AppletsListSearch();

    void init();
    void setFilterModel(QStandardItemModel *model);
    void setItemModel(QStandardItemModel *model);
    QList < AbstractItem * > selectedItems() const;

    /**
     * Insert an appletIcon into the grid layout list
     */
    void insertAppletIcon(int row, int column, PlasmaAppletItem appletItem);

public slots:
    void filterChanged(int index);
    void searchTermChanged(const QString &text);

Q_SIGNALS:

//    void doubleClicked(const QModelIndex &);
//    void clicked(AbstractItem *appletItem);
    void entered(PlasmaAppletItem *appletItem);

private:
    KLineEdit *m_textSearch;
    QGraphicsProxyWidget *m_textSearchProxy;
    AppletsList *m_appletsList;
    QGraphicsProxyWidget *m_appletListProxy;

    friend class WidgetExplorerMainWidget;

};

class AppletsList : public StandardCustomWidget {

    Q_OBJECT;

public:
    AppletsList(QGraphicsItem *parent = 0);
    ~AppletsList();

    void init();

    AbstractItem *getItemByProxyIndex(const QModelIndex &index) const;
    void setItemModel(QStandardItemModel *model);
    void setFilterModel(QStandardItemModel *model);

    void populateAllAppletsHash();

    /**
     * Creates a new applet icon and puts it into the hash
     */
    AppletIconWidget *createAppletIcon(PlasmaAppletItem *appletItem);
    void insertAppletIcon(int row, int column, AppletIconWidget *appletIconWidget);
    void eraseList();

public slots:
    void searchTermChanged(const QString &text);
    void filterChanged(int index);
    void updateList();

private slots:
//    void itemActivated(const QModelIndex &index);
//    void itemDoubleClicked(const QModelIndex &index);
//    void slotSearchTermChanged(const QString &term);


//    void itemClicked(const QModelIndex &index);
//    void itemEntered(const QModelIndex &index);

    //slot to handle the appletIcon and emit SIGNAL appletIconHoverEnter
    void appletIconHoverEnter(AppletIconWidget *appletIcon);

Q_SIGNALS:

    void appletIconHoverEnter(PlasmaAppletItem *item);
    void appletIconHoverLeave(PlasmaAppletItem *item);


private:
    /**
     * Hash containing all widgets that represents the applets
     */
    QHash<QString, AppletIconWidget *> *m_allAppletsHash;

    QGraphicsGridLayout *m_appletListGridLayout;
    QGraphicsWidget *m_appletsListWidget;
    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::PushButton *m_rightArrow;
    Plasma::PushButton *m_leftArrow;

    QStandardItemModel *m_modelItems;
    QStandardItemModel *m_modelFilters;
    DefaultItemFilterProxyModel *m_modelFilterItems;

};


class AppletInfoWidget : public StandardCustomWidget {

    Q_OBJECT;

public:
    explicit AppletInfoWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0, QSizeF constSize = QSize(0,0));
    virtual ~AppletInfoWidget();

    void init();

Q_SIGNALS:
    void infoButtonClicked(const QString &id);

public Q_SLOTS:
    void updateApplet(PlasmaAppletItem *appletItem);
    void onInfoButtonClicked();

private:
    PlasmaAppletItem *m_appletItem;
    QGraphicsWidget *m_form;
    QGraphicsLinearLayout *m_linearLayout;

    Plasma::Label *m_appletDescription;
    AppletIconWidget *m_appletIconWidget;
    Plasma::IconWidget *m_appletInfoButton;
    QSizeF m_constSize;
};

//take this class out of customwidgets
class AppletIconWidget : public Plasma::IconWidget {
    Q_OBJECT;
    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0, bool dotsSurrounded = true);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        PlasmaAppletItem *appletItem();

        //      void addPosition(QPointF position);
//      void updateDots();
        void setAngleBetweenDots(double angle);
        void placeDotsAroundIcon();

    public Q_SLOTS:
        void updateApplet(PlasmaAppletItem *newAppletItem);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);

    private:
        //PlasmaAppletItem m_plasmaAppletItem;
        QList<QPointF> *m_appletPositions;
        QList<PositionDotsSvgWidget> *m_positionDots;

        PlasmaAppletItem *m_appletItem;

       /**
        * radius of sphere where the dots are going to be placed
        */
        float m_dotsSphereRadius;
       /**
        * the minimun angle position to place a dot.
        */
        int m_minAnglePosition;
       /**
        * the maximun angle position to place a dot.
        */
        int m_maxAnglePosition;
        /**
         * the angle between 2 dots - vary according to the y-distance
         */
        mutable double m_angleBetweenDots;
        /**
         * indicates if the icon must be surrounded with applets positions dots
         */
        bool m_dotsSurrounded;

};

class PositionDotsSvgWidget : public Plasma::IconWidget {
    Q_OBJECT;
    public:
        explicit PositionDotsSvgWidget(QGraphicsWidget *parent = 0);
//        virtual ~PositionDotsSvgWidget();

    Q_SIGNALS:
       // removeApplet();

    private:
        QPointF *m_appletPosition;
        double m_anglePosition;
};

class FilteringList : public StandardCustomWidget {

    Q_OBJECT;

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

class ManageWidgetsPushButton : public StandardCustomWidget {

    Q_OBJECT;

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
