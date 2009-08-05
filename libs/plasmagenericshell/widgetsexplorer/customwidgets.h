#ifndef CUSTOMLAYOUTITEMS_H
#define CUSTOMLAYOUTITEMS_H

#include "kcategorizeditemsviewmodels_p.h"
#include "klineedit.h"
#include "plasmaappletitemmodel_p.h"

#include <KMenu>
#include <QBasicTimer>
#include <QTimeLine>
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
#include <plasma/dialog.h>

class AppletIconWidget;
class AppletInfoWidget;
class AppletToolTipWidget;

class AppletsList : public QGraphicsWidget
{

    Q_OBJECT

public:
    AppletsList(QGraphicsItem *parent = 0);
    ~AppletsList();

    QList < KCategorizedItemsViewModels::AbstractItem * > selectedItems() const;
    void setItemModel(QStandardItemModel *model);
    void setFilterModel(QStandardItemModel *model);

private:
    void init();

    KCategorizedItemsViewModels::AbstractItem *getItemByProxyIndex(const QModelIndex &index) const;

    void populateAllAppletsHash();

    /**
     * Creates a new applet icon and puts it into the hash
     */
    AppletIconWidget *createAppletIcon(PlasmaAppletItem *appletItem);
    void insertAppletIcon(AppletIconWidget *appletIconWidget);

    qreal listWidth();
    int maximumVisibleIconsOnList();
    void eraseList();
    void setToolTipPosition();

    AppletIconWidget *findAppletUnderXPosition(int xPosition);
    QRectF visibleListRect();

    void scroll(bool right, bool byWheel);
    void scrollRight(int step, QRectF visibleRect);
    void scrollLeft(int step, QRectF visibleRect);
    void wheelEvent(QGraphicsSceneWheelEvent *event);

private slots:
    void searchTermChanged(const QString &text);
    void filterChanged(int index);
    void updateList();

    void onRightArrowClick();
    void onLeftArrowClick();
    void manageArrows();
    void resetScroll();

    void itemSelected(AppletIconWidget *applet);
    void appletIconDoubleClicked(AppletIconWidget *applet);
    void appletIconHoverLeave(AppletIconWidget *appletIcon);
    void appletIconHoverEnter(AppletIconWidget *appletIcon);
    void onToolTipEnter();
    void onToolTipLeave();

    /* TODO: Remove this and animate using plasma's
     * animation framework when it is created */
    void animateMoveBy(int amount);
    void scrollTimeLineFrameChanged(int frame);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void timerEvent(QTimerEvent *event);

Q_SIGNALS:

    void appletDoubleClicked(PlasmaAppletItem *appletItem);
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

    /**
     * One single tootip to show applets info
     */
    AppletToolTipWidget *m_toolTip;

    QStandardItemModel *m_modelItems;
    QStandardItemModel *m_modelFilters;
    KCategorizedItemsViewModels::DefaultItemFilterProxyModel *m_modelFilterItems;

    AppletIconWidget *m_selectedItem;

    QBasicTimer m_toolTipAppearTimer;
    QBasicTimer m_toolTipDisappearTimer;
    QBasicTimer m_searchDelayTimer;
    QString m_searchString;

    int arrowClickStep;
    int scrollStep;

    /* TODO: Remove this and animate using plasma's
     * animation framework when it is created */
    QTimeLine scrollTimeLine;
    qreal scrollTo;
    qreal scrollFrom;
};

class AppletIconWidget : public Plasma::IconWidget
{

    Q_OBJECT

    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        void setSelected(bool selected);
        PlasmaAppletItem *appletItem();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public Q_SLOTS:
        void updateApplet(PlasmaAppletItem *newAppletItem);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);
        void selected(AppletIconWidget *applet);
        void doubleClicked(AppletIconWidget *applet);

    private:
        PlasmaAppletItem *m_appletItem;
        bool m_selected;
        bool m_hovered;
        Plasma::FrameSvg *m_selectedBackgroundSvg;
        bool m_showingTooltip;
};

class AppletToolTipWidget : public Plasma::Dialog {

    Q_OBJECT

    public:
        explicit AppletToolTipWidget(QWidget *parent = 0, AppletIconWidget *applet = 0);
        virtual ~AppletToolTipWidget();

        void setAppletIconWidget(AppletIconWidget *applet);
        void updateContent();
        AppletIconWidget *appletIconWidget();

    Q_SIGNALS:
        void enter();
        void leave();

    protected:
        void enterEvent(QEvent *event);
        void leaveEvent(QEvent *event);

    private:        
        AppletIconWidget *m_applet;
        AppletInfoWidget *m_widget;
};

class AppletInfoWidget : public QGraphicsWidget {

    Q_OBJECT

    public:
        AppletInfoWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        ~AppletInfoWidget();

        void init();
        void setAppletItem(PlasmaAppletItem *appletItem);

    private:
        void fixSizes();

    public Q_SLOTS:
        void updateInfo();

    private:
        PlasmaAppletItem *m_appletItem;
        QGraphicsLinearLayout *m_linearLayout;

        Plasma::Label *m_descriptionLabel;
        Plasma::IconWidget *m_iconWidget;
        Plasma::IconWidget *m_infoButton;
        Plasma::Label *m_nameLabel;
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
