#ifndef CUSTOMLAYOUTITEMS_H
#define CUSTOMLAYOUTITEMS_H

#include "kcategorizeditemsviewmodels_p.h"
#include "klineedit.h"
#include "plasmaappletitemmodel_p.h"

#include <KMenu>
#include <QBasicTimer>
#include <QScrollArea>
#include <QtCore>
#include <QtGui>

#include <kpushbutton.h>

#include <plasma/framesvg.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/pushbutton.h>
#include <plasma/widgets/treeview.h>

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
    double listWidth();
    void eraseList();
    void scroll(bool right, bool byWheel);
    void scrollRight(int step, QRectF visibleRect);
    void scrollLeft(int step, QRectF visibleRect);
    void wheelEvent(QGraphicsSceneWheelEvent *event);

    QList < KCategorizedItemsViewModels::AbstractItem * > selectedItems() const;
    AppletIconWidget *findAppletUnderXPosition(int xPosition);

public slots:
    void searchTermChanged(const QString &text);
    void filterChanged(int index);
    void updateList();
    void appletIconEnter(AppletIconWidget *appletIcon);
    void onRightArrowClick();
    void onLeftArrowClick();
    void manageArrows();
    void resetScroll();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void timerEvent(QTimerEvent *event);

Q_SIGNALS:

    void appletIconHoverEnter(PlasmaAppletItem *item);
    void appletIconHoverLeave(PlasmaAppletItem *item);

    void appletIconEnter(PlasmaAppletItem *appletItem);
    void listScrolled();


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

    QBasicTimer m_searchDelayTimer;
    QString m_searchString;

    int arrowClickStep;
    int scrollStep;
};

class AppletIconWidget : public Plasma::IconWidget
{

    Q_OBJECT

    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        PlasmaAppletItem *appletItem();
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    public Q_SLOTS:
        void updateApplet(PlasmaAppletItem *newAppletItem);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);

    private:
        PlasmaAppletItem *m_appletItem;

        bool selected;
        Plasma::FrameSvg *m_selectedBackgroundSvg;

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
