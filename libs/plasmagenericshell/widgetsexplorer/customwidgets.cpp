#include "customwidgets.h"
#include <KAction>
#include <KStandardAction>
#include <kiconloader.h>
#include <cmath>
#include <QHash>
#include <typeinfo>

#include <plasma/tooltipmanager.h>

#define UNIVERSAL_PADDING 20
#define SEARCH_DELAY 300
#define ICON_WIDGET_HEIGHT 100
#define ICON_WIDGET_WIDTH 100

//AppletsList

using namespace KCategorizedItemsViewModels;

AppletsList::AppletsList(QGraphicsItem *parent)
        :QGraphicsWidget(parent)
{
    init();
    arrowClickStep = 0;
    scrollStep = 0;
    m_selectedItem = 0;
    connect(this, SIGNAL(listScrolled()), this, SLOT(manageArrows()));

    scrollTimeLine.setFrameRange(0, 100);
    scrollTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);
    scrollTimeLine.setDuration(500); // TODO: Set this to a lesser value
    connect(&scrollTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(scrollTimeLineFrameChanged(int)));
}

AppletsList::~AppletsList()
{
}

void AppletsList::init()
{
    m_leftArrow = new Plasma::PushButton();
    m_leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    m_leftArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_leftArrow->setAttribute(Qt::WA_NoSystemBackground);

    m_rightArrow = new Plasma::PushButton();
    m_rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    m_rightArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(onRightArrowClick()));
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(onLeftArrowClick()));

    m_appletsListWidget = new QGraphicsWidget();
    m_appletListLinearLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_appletsListWidget->setLayout(m_appletListLinearLayout);

    m_appletsListWindowWidget = new QGraphicsWidget();
    m_appletsListWindowWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_appletsListWidget->setParentItem(m_appletsListWindowWidget);

    m_arrowsLayout = new QGraphicsLinearLayout();

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_appletsListWindowWidget);
    m_arrowsLayout->addItem(m_rightArrow);

    m_arrowsLayout->setAlignment(m_rightArrow, Qt::AlignRight);
    m_arrowsLayout->setAlignment(m_leftArrow, Qt::AlignLeft);

    m_arrowsLayout->setAlignment(m_rightArrow, Qt::AlignVCenter);
    m_arrowsLayout->setAlignment(m_leftArrow, Qt::AlignVCenter);

    setLayout(m_arrowsLayout);

}

void AppletsList::resizeEvent(QGraphicsSceneResizeEvent *event) {
    Q_UNUSED(event)
    m_appletsListWindowWidget->setMinimumHeight(m_appletsListWidget->geometry().height());
    m_appletsListWindowWidget->setMaximumHeight(m_appletsListWidget->geometry().height());

    int maxVisibleIconsOnList = maximumVisibleIconsOnList();
    arrowClickStep = ceil(maxVisibleIconsOnList/4);
    scrollStep = ceil(maxVisibleIconsOnList/2);
}

void AppletsList::setItemModel(QStandardItemModel *model)
{
    m_modelFilterItems = new DefaultItemFilterProxyModel(this);

    m_modelItems = model;
    m_modelFilterItems->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_modelFilterItems->setDynamicSortFilter(true);
    m_modelFilterItems->setSourceModel(m_modelItems);
    m_modelFilterItems->sort(0);

    populateAllAppletsHash();

    connect(m_modelFilterItems, SIGNAL(searchTermChanged(QString)), this, SLOT(updateList()));
    connect(m_modelFilterItems, SIGNAL(filterChanged()), this, SLOT(updateList()));

    updateList();

}

void AppletsList::setFilterModel(QStandardItemModel *model)
{
    m_modelFilters = model;
}

void AppletsList::filterChanged(int index)
{
    if (m_modelFilterItems) {
        QVariant data = m_modelFilters->item(index)->data();
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>(data));
    }
}

void AppletsList::searchTermChanged(const QString &text)
{
    m_searchString = text;
    m_searchDelayTimer.start(SEARCH_DELAY, this);
}

void AppletsList::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_searchDelayTimer.timerId()) {
        m_modelFilterItems->setSearch(m_searchString);
        m_searchDelayTimer.stop();
    }
    QGraphicsWidget::timerEvent(event);
}

void AppletsList::insertAppletIcon(AppletIconWidget *appletIconWidget)
{
    appletIconWidget->setVisible(true);
    m_appletListLinearLayout->addItem(appletIconWidget);
}

qreal AppletsList::listWidth() {
    return m_appletsListWidget->childItems().count() *
                    (ICON_WIDGET_WIDTH + m_appletListLinearLayout->spacing()) -
                    m_appletListLinearLayout->spacing();
}

int AppletsList::maximumVisibleIconsOnList() {
    qreal windowWidth = m_appletsListWindowWidget->geometry().width();
    qreal maxVisibleIconsOnList = floor(windowWidth/(ICON_WIDGET_WIDTH
                                                          + m_appletListLinearLayout->spacing()));
    return maxVisibleIconsOnList;
}

AppletIconWidget *AppletsList::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget(0, appletItem);
//    ***** set this another way ********
    applet->setMinimumSize(ICON_WIDGET_WIDTH, ICON_WIDGET_HEIGHT);
    applet->setMaximumSize(ICON_WIDGET_WIDTH, ICON_WIDGET_HEIGHT);

    connect(applet, SIGNAL(hoverEnter(AppletIconWidget*)), this, SLOT(appletIconEnter(AppletIconWidget*)));
    connect(applet, SIGNAL(selected(AppletIconWidget*)), this, SLOT(itemSelected(AppletIconWidget*)));
    connect(applet, SIGNAL(doubleClicked(AppletIconWidget*)), this, SLOT(appletIconDoubleClicked(AppletIconWidget*)));

    return applet;
}

void AppletsList::itemSelected(AppletIconWidget *applet)
{
    if(m_selectedItem) {
        m_selectedItem->setSelected(false);
    }
    applet->setSelected(true);
    m_selectedItem = applet;
}

void AppletsList::appletIconDoubleClicked(AppletIconWidget *applet)
{
    emit(appletDoubleClicked(applet->appletItem()));
}

void AppletsList::eraseList() {
    QList<QGraphicsItem *> applets = m_appletsListWidget->childItems();
    foreach(QGraphicsItem *applet, applets) {
        applet->setParentItem(0);
        applet->setVisible(false);
    }
}

void AppletsList::updateList()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    AppletIconWidget *appletIconWidget;

    m_appletsListWidget->setLayout(NULL);
    m_appletListLinearLayout = new QGraphicsLinearLayout();
    m_appletListLinearLayout->setContentsMargins(0, 0, 0, 0);

    eraseList();

    for(int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;

        if(appletItem != 0) {
            appletIconWidget = m_allAppletsHash->value(appletItem->id());
            insertAppletIcon(appletIconWidget);
        }
    }

    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    m_appletListLinearLayout->setSpacing(10);
    resetScroll();
}

void AppletsList::wheelEvent(QGraphicsSceneWheelEvent *event) {
    //if the scroll is right or left
    bool right;
    bool byWheel = true;
    bool canScroll;

    right = event->delta() < 0 ? true : false;

    if(right) {
        canScroll = m_rightArrow->isEnabled() ? true : false;
    } else {
        canScroll = m_leftArrow->isEnabled() ? true : false;
    }

    if(canScroll) {
        scroll(right, byWheel);
    }
}

void AppletsList::onRightArrowClick() {
    scroll(true, false);
}

void AppletsList::onLeftArrowClick() {
    scroll(false, false);
}

void AppletsList::scroll(bool right, bool byWheel) {
    QRectF visibleRect = visibleListRect();
    int step = byWheel ? scrollStep : arrowClickStep;

    if(right) {
        scrollRight(step, visibleRect);
    } else {
        scrollLeft(step, visibleRect);
    }
}

void AppletsList::scrollRight(int step, QRectF visibleRect) {
    AppletIconWidget *clipped;
    int newFirstVisibleXOnList;
    int newFirstVisibleXOnWindow;
    qreal scrollXAmount;
    int lastVisibleXOnList = visibleRect.x() + visibleRect.width();
    int listWidthVar = listWidth();

    clipped = findAppletUnderXPosition(lastVisibleXOnList);

    if(clipped) {
        newFirstVisibleXOnList = (clipped->pos().x() +
                                     clipped->boundingRect().width()) -
                                    m_appletsListWindowWidget->boundingRect().width();
        newFirstVisibleXOnWindow = (m_appletsListWidget->mapToItem(m_appletsListWindowWidget,
                                                                   newFirstVisibleXOnList, 0)).x();
        scrollXAmount = newFirstVisibleXOnWindow +
                                          (step - 1) * (m_appletListLinearLayout->spacing() +
                                               ICON_WIDGET_WIDTH);
    } else {
        scrollXAmount = step * (m_appletListLinearLayout->spacing() +
                                               ICON_WIDGET_WIDTH);
    }

    //check if the scrollAmount is more than necessary to reach the end of the list
    if(lastVisibleXOnList + scrollXAmount > listWidthVar) {
        scrollXAmount = listWidthVar - lastVisibleXOnList;
    }

    // m_appletsListWidget->moveBy(-scrollXAmount, 0);
    animateMoveBy(-scrollXAmount);
    emit(listScrolled());
}

void AppletsList::scrollLeft(int step, QRectF visibleRect) {
    qreal firstVisibleXOnList = visibleRect.x();
    AppletIconWidget *clipped;
    int newFirstVisibleXOnWindow;
    qreal scrollXAmount;

    clipped = findAppletUnderXPosition(firstVisibleXOnList);

    if(clipped) {
        newFirstVisibleXOnWindow = (m_appletsListWidget->mapToItem(m_appletsListWindowWidget,
                                                                   clipped->pos().x(), 0)).x();
        scrollXAmount =  (-newFirstVisibleXOnWindow) + ((step - 1) *
                                                     (m_appletListLinearLayout->spacing() +
                                                      ICON_WIDGET_WIDTH));
    } else {
        scrollXAmount = (step * (m_appletListLinearLayout->spacing() +
                                               ICON_WIDGET_WIDTH));
    }

    //check if the scrollAmount is more than necessary to reach the begining of the list
    if(firstVisibleXOnList - scrollXAmount <  0){
        scrollXAmount = firstVisibleXOnList;
    }

    //m_appletsListWidget->moveBy(scrollXAmount, 0);
    animateMoveBy(scrollXAmount);
    emit(listScrolled());
}

/* ivan: Implementation of the following functions is
 * less than optimal and is intended just to provide
 * the temporary solution until plasma gets the new
 * animation framework.
 * TODO: Remove this and animate using plasma's
 * animation framework when it is created */
void AppletsList::animateMoveBy(int amount)
{
    scrollFrom = m_appletsListWidget->pos().x();
    if (scrollTimeLine.state() == QTimeLine::Running) {
        scrollTo = scrollTo + amount;
    } else {
        scrollTo = scrollFrom + amount;
    }

    if (scrollTo > 0) {
        scrollTo = 0;
    }

    if (scrollTo + m_appletsListWidget->size().width() <
        m_appletsListWindowWidget->size().width()) {
        scrollTo = m_appletsListWindowWidget->size().width()
                   - m_appletsListWidget->size().width();
    }

    // scrollTimeLine.stop();

    if (scrollTimeLine.state() != QTimeLine::Running &&
            scrollFrom != scrollTo) {
        scrollTimeLine.start();
    }
}

void AppletsList::scrollTimeLineFrameChanged(int frame)
{
    QPointF newPos = QPointF(
        (frame/(qreal)100) * (scrollTo - scrollFrom) + scrollFrom,
        m_appletsListWidget->pos().y());
    m_appletsListWidget->setPos(newPos);

    if (frame == 100) {
        manageArrows();
    }
}

void AppletsList::resetScroll() {
    m_appletsListWidget->setPos(0,0);
    manageArrows();
}

void AppletsList::manageArrows() {
    QRectF visibleRect;
    int firstVisibleXOnList;
    int lastVisibleXOnList;

    qreal listWidthVar = listWidth();

    if(listWidthVar <= m_appletsListWindowWidget->geometry().width()) {
        m_leftArrow->setEnabled(false);
        m_rightArrow->setEnabled(false);

    } else {
        visibleRect = visibleListRect();
        firstVisibleXOnList = visibleRect.x();
        lastVisibleXOnList = firstVisibleXOnList + visibleRect.width();

        if(firstVisibleXOnList <= 0) {
            m_leftArrow->setEnabled(false);
        } else {
            m_leftArrow->setEnabled(true);
        }

        if(lastVisibleXOnList >= listWidthVar) {
            m_rightArrow->setEnabled(false);
        } else {
            m_rightArrow->setEnabled(true);
        }
    }
}

AppletIconWidget *AppletsList::findAppletUnderXPosition(int xPosition) {
    QList<QGraphicsItem *> applets = m_appletsListWidget->childItems();

    int firstXOnApplet;
    int lastXOnApplet;

    foreach(QGraphicsItem *applet, applets) {
        firstXOnApplet = applet->mapToItem(m_appletsListWidget, 0, 0).x();
        lastXOnApplet = applet->mapToItem(m_appletsListWidget, applet->boundingRect().width(), 0).x();
        if((xPosition > firstXOnApplet) && (xPosition < lastXOnApplet)) {
            return dynamic_cast<AppletIconWidget*>(applet);
        }
    }
    return 0;
}

QRectF AppletsList::visibleListRect() {
    QRectF visibleRect = m_appletsListWindowWidget->
                         mapRectToItem(m_appletsListWidget, 0, 0,
                                          m_appletsListWindowWidget->geometry().width(),
                                          m_appletsListWindowWidget->geometry().height());

    return visibleRect;
}

void AppletsList::populateAllAppletsHash()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    int indexesCount = m_modelFilterItems->rowCount();

    m_allAppletsHash = new QHash<QString, AppletIconWidget *>();

    for(int i = 0; i < indexesCount ; i++){
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;
        m_allAppletsHash->insert(appletItem->id(), createAppletIcon(appletItem));
    }
}

AbstractItem *AppletsList::getItemByProxyIndex(const QModelIndex &index) const
{
    return (AbstractItem *)m_modelItems->itemFromIndex(m_modelFilterItems->mapToSource(index));
}

void AppletsList::appletIconEnter(AppletIconWidget *appletIcon)
{
    emit(appletIconEnter(appletIcon->appletItem()));
}

QList <AbstractItem *> AppletsList::selectedItems() const
{
//    return m_appletList->selectedItems();
    return QList<AbstractItem *>();
}

//AppletIconWidget

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
    : Plasma::IconWidget(parent)
{
    m_appletItem = appletItem;
    m_hovered = false;
    m_selected = false;
    m_selectedBackgroundSvg = new Plasma::FrameSvg(this);
    m_selectedBackgroundSvg->setImagePath("widgets/translucentbackground");

    updateApplet(appletItem);
}

AppletIconWidget::~AppletIconWidget()
{
    m_appletItem = 0;
    Plasma::ToolTipManager::self()->unregisterWidget(this);
}

PlasmaAppletItem *AppletIconWidget::appletItem()
{
    return m_appletItem;
}

void AppletIconWidget::setAppletItem(PlasmaAppletItem *appletIcon)
{
   m_appletItem = appletIcon;
}

void AppletIconWidget::updateApplet(PlasmaAppletItem *appletItem)
{
    Plasma::ToolTipContent data;
    QPixmap p;

    if(appletItem != 0) {
        m_appletItem = appletItem;
        setText(m_appletItem->name());
        setIcon(m_appletItem->icon());

        data = Plasma::ToolTipContent();

        p = m_appletItem->icon().pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);

        data.setMainText(m_appletItem->name());
        data.setImage(p);
        data.setSubText(m_appletItem->description());

        Plasma::ToolTipManager::self()->setContent(this, data);

    } else {
        setText("no name");
        setIcon("widgets/clock");
    }
}

void AppletIconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverEnterEvent(event);
    m_hovered = true;
    emit(hoverEnter(this));
}

void AppletIconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverLeaveEvent(event);
    m_hovered = false;
    emit(hoverLeave(this));
}

void AppletIconWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    Plasma::IconWidget::mouseMoveEvent(event);
    if (event->button() != Qt::LeftButton
        && (event->pos() - event->buttonDownPos(Qt::LeftButton))
            .toPoint().manhattanLength() > QApplication::startDragDistance()
    ) {
        event->accept();
        qDebug() << "Start Dragging";
        QDrag *drag = new QDrag(event->widget());
        QPixmap p = appletItem()->icon().pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
        drag->setPixmap(p);

        QMimeData *data = m_appletItem->mimeData();

        drag->setMimeData(data);
        drag->exec();

        mouseReleaseEvent(event);
    }
}

void AppletIconWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit(selected(this));
}

void AppletIconWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit(doubleClicked(this));
}

void AppletIconWidget::setSelected(bool selected)
{
    m_selected = selected;
    update(0,0,boundingRect().width(), boundingRect().height());
}

void AppletIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
    if(m_selected || m_hovered) {
        m_selectedBackgroundSvg->resizeFrame(boundingRect().size());
        m_selectedBackgroundSvg->paintFrame(painter, boundingRect().topLeft());
        if(m_selected) {
            //again
            m_selectedBackgroundSvg->paintFrame(painter, boundingRect().topLeft());
        }
     }

    Plasma::IconWidget::paint(painter, option, widget);
 }

//FilteringList

FilteringList::FilteringList(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    init();
    connect(m_treeView->nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(filterChanged(const QModelIndex &)));
}

FilteringList::~FilteringList()
{
}

void FilteringList::init()
{
    m_treeView = new Plasma::TreeView();
    m_treeView->nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    m_treeView->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->nativeWidget()->setRootIsDecorated(false);
    m_treeView->nativeWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_treeView->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout();
    linearLayout->addItem(m_treeView);
    setLayout(linearLayout);
    m_treeView->nativeWidget()->header()->setVisible(false);
    m_treeView->nativeWidget()->setPalette((new Plasma::IconWidget())->palette());
}

void FilteringList::setModel(QStandardItemModel *model)
{
    m_model = model;
    m_treeView->setModel(m_model);
}


void FilteringList::filterChanged(const QModelIndex & index)
{
    emit(filterChanged(index.row()));
}

//FilteringWidget

FilteringWidget::FilteringWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    m_backgroundSvg = new Plasma::FrameSvg(this);
    m_backgroundSvg->setImagePath("widgets/background");

    init();
}

FilteringWidget::~FilteringWidget(){
}

void FilteringWidget::init()
{
    m_filterLabel = new Plasma::Label();
    m_filterLabel->nativeWidget()->setText("Filter");
    m_filterLabel->setMaximumHeight(10);
    m_filterLabel->setMinimumHeight(10);

    QFont labelFont = m_filterLabel->font();
    labelFont.setPointSize(3);
    m_filterLabel->setFont(labelFont);

    m_textSearch = new Plasma::LineEdit();
    m_textSearch->nativeWidget()->setClickMessage(/*i18n(*/"Type search"/*)*/);
    m_textSearch->setFocus();
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setMaximumHeight(30);
    m_textSearch->setMinimumHeight(30);

    m_categoriesList = new FilteringList();

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(Qt::Vertical);

    linearLayout->addItem(m_filterLabel);
    linearLayout->addItem(m_textSearch);
    linearLayout->addItem(m_categoriesList);

    this->setLayout(linearLayout);
    linearLayout->setContentsMargins(15,15,15,15);
}

void FilteringWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_backgroundSvg->resizeFrame(contentsRect().size());
     m_backgroundSvg->paintFrame(painter, contentsRect().topLeft());
 }

FilteringList *FilteringWidget::categoriesList()
{
    return m_categoriesList;
}

Plasma::LineEdit *FilteringWidget::textSearch()
{
    return m_textSearch;
}

//ManageWidgetsPushButton

ManageWidgetsPushButton::ManageWidgetsPushButton(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    init();
}

ManageWidgetsPushButton::~ManageWidgetsPushButton(){
}


void ManageWidgetsPushButton::init()
{
    m_button = new KPushButton();
    m_button->setFlat(true);
    m_button->setAttribute(Qt::WA_NoSystemBackground);
    m_buttonProxy = new QGraphicsProxyWidget();
    m_buttonProxy->setWidget(m_button);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_buttonProxy);
    layout->setAlignment(m_buttonProxy, Qt::AlignVCenter);
    setLayout(layout);

}

KPushButton *ManageWidgetsPushButton::button()
{
    return m_button;
}

KMenu *ManageWidgetsPushButton::buttonMenu()
{
    return m_buttonMenu;
}


QGraphicsProxyWidget *ManageWidgetsPushButton::buttonProxy()
{
    return m_buttonProxy;
}

