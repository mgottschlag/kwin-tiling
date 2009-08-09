#ifndef APPLETSLIST_H
#define APPLETSLIST_H

#include <QBasicTimer>
#include <QTimeLine>
#include <QtCore>
#include <QtGui>

#include <plasma/widgets/pushbutton.h>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "appleticon.h"
#include "applettooltip.h"

class AppletsList : public QGraphicsWidget
{

    Q_OBJECT

public:
    AppletsList(Qt::Orientation orientation = Qt::Horizontal, QGraphicsItem *parent = 0);
    ~AppletsList();

    QList < KCategorizedItemsViewModels::AbstractItem * > selectedItems() const;
    void setItemModel(QStandardItemModel *model);
    void setFilterModel(QStandardItemModel *model);
    void setOrientation(Qt::Orientation orientation);

    enum ScrollPolicy {
        Right = 0,
        Left = 1,
        Up = 2,
        Down = 3,
        Wheel = 4,
        Button = 5
    };

private:
    void init();

    KCategorizedItemsViewModels::AbstractItem *getItemByProxyIndex(const QModelIndex &index) const;

    void populateAllAppletsHash();

    /**
     * Creates a new applet icon and puts it into the hash
     */
    AppletIconWidget *createAppletIcon(PlasmaAppletItem *appletItem);
    void insertAppletIcon(AppletIconWidget *appletIconWidget);

    int maximumAproxVisibleIconsOnList();
    void eraseList();
    void setToolTipPosition();

    bool isItemUnder(int itemIndex, qreal xPosition);
    int findFirstVisibleApplet(int firstVisibleXOnList);
    int findLastVisibleApplet(int lastVisibleXOnList);
    QRectF visibleListRect();

    void scroll(ScrollPolicy side, ScrollPolicy how);
    void scrollRight(int step, QRectF visibleRect);
    void scrollLeft(int step, QRectF visibleRect);
    void wheelEvent(QGraphicsSceneWheelEvent *event);

    void adjustContentsAccordingToOrientation();

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
    void infoButtonClicked(const QString &apluginName);

private:

    /**
     * Hash containing all widgets that represents the applets
     */
    QHash<QString, AppletIconWidget *> *m_allAppletsHash;
    QList<AppletIconWidget *> *m_currentAppearingAppletsOnList;

    QGraphicsLinearLayout *m_appletListLinearLayout;
    QGraphicsWidget *m_appletsListWidget;
    QGraphicsWidget *m_appletsListWindowWidget;
    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::PushButton *m_downRightArrow;
    Plasma::PushButton *m_upLeftArrow;

    Qt::Orientation m_orientation;

    /**
     * One single tootip to show applets info
     */
    AppletToolTipWidget *m_toolTip;

    QStandardItemModel *m_modelItems;
    QStandardItemModel *m_modelFilters;
    KCategorizedItemsViewModels::DefaultItemFilterProxyModel *m_modelFilterItems;

    AppletIconWidget *m_selectedItem;

    QVariant m_dataFilterAboutToApply;
    QBasicTimer m_filterApplianceTimer;
    QBasicTimer m_toolTipAppearTimer;
    QBasicTimer m_toolTipDisappearTimer;
    QBasicTimer m_searchDelayTimer;
    QString m_searchString;

    int arrowClickStep;
    int wheelStep;

    /* TODO: Remove this and animate using plasma's
     * animation framework when it is created */
    QTimeLine scrollTimeLine;
    qreal scrollTo;
    qreal scrollFrom;

};

#endif //APPLETSLIST_H
