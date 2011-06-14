/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "abstracttaskitem.h"

// Qt
#include <QApplication>
#include <QFileInfo>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QTextLayout>
#include <QTimer>
#include <QVarLengthArray>
#include <QPropertyAnimation>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

// KDE
#include <KAuthorized>
#include <KColorUtils>
#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconEffect>
#include <KIconLoader>

#include <NETWinInfo>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <Plasma/WindowEffects>

#include <taskmanager/task.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>

#include "tasks.h"
#include "taskgroupitem.h"

static const int HOVER_EFFECT_TIMEOUT = 900;

AbstractTaskItem::AbstractTaskItem(QGraphicsWidget *parent, Tasks *applet)
    : QGraphicsWidget(parent),
      m_abstractItem(0),
      m_applet(applet),
      m_flags(0),
      m_backgroundFadeAnim(0),
      m_alpha(1),
      m_backgroundPrefix("normal"),
      m_activateTimerId(0),
      m_updateGeometryTimerId(0),
      m_updateTimerId(0),
      m_hoverEffectTimerId(0),
      m_attentionTimerId(0),
      m_attentionTicks(0),
      m_lastViewId(0),
      m_showText(true),
      m_layoutAnimationLock(false),
      m_firstGeometryUpdate(false)
{
    m_layoutAnimation = new QPropertyAnimation(this, "animationPos", this);
    m_layoutAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_layoutAnimation->setDuration(250);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setFlag(QGraphicsItem::ItemIsFocusable);

    checkSettings();
    connect(applet->itemBackground(), SIGNAL(repaintNeeded()), this, SLOT(syncActiveRect()));
    connect(applet, SIGNAL(settingsChanged()), this, SLOT(checkSettings()));
}

QSize AbstractTaskItem::basicPreferredSize() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    const int iconsize = KIconLoader::SizeSmall;

    //the 4 should be the default spacing between layout items, is there a way to fetch it without hardcoding?
    // in small panels, we'll reduce the spacing a bit so it's easier to cramp the text in and still get two rows
    int topMargin = m_applet->itemTopMargin();
    int bottomMargin = m_applet->itemBottomMargin();

    //if this item is out of the applet rect must be in a popup
    //const bool inPopup = (m_applet->mapToScene(m_applet->rect()).boundingRect().contains(mapToScene(rect()).boundingRect()))

    if (m_applet->size().height() < 44) {
        topMargin = 1;
        bottomMargin = 1;
    } else if (m_applet->size().height() < 64) {
        topMargin = qMax(1, topMargin/2); 
        bottomMargin = qMax(1, bottomMargin/2); 
    }

    //kDebug() << (QObject*)this;
    return QSize(mSize.width()*12 + m_applet->itemLeftMargin() + m_applet->itemRightMargin() + KIconLoader::SizeSmall,
            qMax(mSize.height(), iconsize) + topMargin + bottomMargin);
}

void AbstractTaskItem::setPreferredOffscreenSize()
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    QSize mSize = fm.size(0, "M");
    int iconsize = KIconLoader::SizeSmall;
    int topMargin = m_applet->offscreenTopMargin();
    int bottomMargin = m_applet->offscreenBottomMargin();
    int rightMargin = m_applet->offscreenRightMargin();
    int leftMargin = m_applet->offscreenLeftMargin();

    //kDebug() << (QObject*)this;
    QSizeF s(mSize.width() * 12 + leftMargin + rightMargin + KIconLoader::SizeSmall,
             qMax(mSize.height(), iconsize) + topMargin + bottomMargin);
    setPreferredSize(s);
}

void AbstractTaskItem::setPreferredOnscreenSize()
{
    setPreferredSize(basicPreferredSize());
}

AbstractTaskItem::~AbstractTaskItem()
{
    stopWindowHoverEffect();
    emit destroyed(this);
    Plasma::ToolTipManager::self()->unregisterWidget(this);
}

void AbstractTaskItem::checkSettings()
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem *>(this);

    if (m_applet->showToolTip() && (!group || group->collapsed())) {
        clearToolTip();
    } else {
        Plasma::ToolTipManager::self()->unregisterWidget(this);
    }
}

void AbstractTaskItem::clearToolTip()
{
    Plasma::ToolTipContent data;
    data.setInstantPopup(true);

    Plasma::ToolTipManager::self()->setContent(this, data);
}

void AbstractTaskItem::clearAbstractItem()
{
    m_abstractItem = 0;
}

void AbstractTaskItem::textChanged()
{
    m_cachedShadow = QPixmap();
}

QString AbstractTaskItem::text() const
{
    if (m_abstractItem) {
        return m_abstractItem->name();
    } else {
        kDebug() << "no abstract item?";
    }

    return QString();
}

QIcon AbstractTaskItem::icon() const
{
    if (m_abstractItem) {
        return m_abstractItem->icon();
    }

    return QIcon();
}

void AbstractTaskItem::setTaskFlags(const TaskFlags flags)
{
    if (((m_flags & TaskWantsAttention) != 0) != ((flags & TaskWantsAttention) != 0)) {
        //kDebug() << "task attention state changed" << m_attentionTimerId;
        m_flags = flags;
        if (flags & TaskWantsAttention) {
            m_applet->needsVisualFocus(true);
            // start attention getting
            if (!m_attentionTimerId) {
                m_attentionTimerId = startTimer(500);
            }
        } else {
            m_applet->needsVisualFocus(false);
            if (m_attentionTimerId) {
                killTimer(m_attentionTimerId);
                m_attentionTimerId = 0;
            }
        }
    }

    m_flags = flags;

    QString newBackground;
    if (m_flags & TaskIsMinimized) {
        newBackground = "minimized";
    } else if (m_flags & TaskHasFocus) {
        newBackground = "focus";
    } else {
        newBackground = "normal";
    }

    if (newBackground != m_backgroundPrefix) {
        fadeBackground(newBackground, 250);
    }
}

void AbstractTaskItem::fadeBackground(const QString &newBackground, int duration)
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem*>(this);
    if (group && !group->collapsed()) {
        return;
    }

    m_oldBackgroundPrefix = m_backgroundPrefix;
    m_backgroundPrefix = newBackground;

    if (m_oldBackgroundPrefix.isEmpty()) {
        update();
    } else {
        if (!m_backgroundFadeAnim) {
            m_backgroundFadeAnim = new QPropertyAnimation(this);
            m_backgroundFadeAnim->setDuration(duration);
            m_backgroundFadeAnim->setEasingCurve(QEasingCurve::InQuad);
            m_backgroundFadeAnim->setPropertyName("backgroundFadeAlpha");
            m_backgroundFadeAnim->setTargetObject(this);
            m_backgroundFadeAnim->setStartValue(0);
            m_backgroundFadeAnim->setEndValue(1);
        }

        m_backgroundFadeAnim->start();
    }
}

AbstractTaskItem::TaskFlags AbstractTaskItem::taskFlags() const
{
    return m_flags;
}

void AbstractTaskItem::toolTipAboutToShow()
{
    if (m_applet->showToolTip()) {
        updateToolTip();
        connect(Plasma::ToolTipManager::self(),
                SIGNAL(windowPreviewActivated(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)),
                this, SLOT(activateWindow(WId,Qt::MouseButtons)));
    } else {
        clearToolTip();
    }
}

void AbstractTaskItem::toolTipHidden()
{
    clearToolTip();
    disconnect(Plasma::ToolTipManager::self(),
               SIGNAL(windowPreviewActivated(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)),
               this, SLOT(activateWindow(WId,Qt::MouseButtons)));
}

void AbstractTaskItem::activateWindow(WId id, Qt::MouseButtons buttons)
{
    if (buttons & Qt::LeftButton) {
        if (parentGroup()) {
            AbstractTaskItem *item = parentGroup()->taskItemForWId(id);
            if (item) {
                item->activate();
            }
        }
    }
}

void AbstractTaskItem::queueUpdate()
{
    if (m_updateTimerId || m_attentionTimerId) {
        return;
    }

    if (m_lastUpdate.elapsed() < 100) {
        m_updateTimerId = startTimer(100);
        return;
    }

    update();
    m_lastUpdate.restart();
}

void AbstractTaskItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    setTaskFlags(m_flags | TaskHasFocus);
    update();
}

void AbstractTaskItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    setTaskFlags(m_flags & ~TaskHasFocus);
    update();
}

void AbstractTaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    fadeBackground("hover", 250);
    QGraphicsWidget *w = parentWidget();
    if (w && this != m_applet->rootGroupItem()) {
        if (m_hoverEffectTimerId) {
            killTimer(m_hoverEffectTimerId);
            m_hoverEffectTimerId = 0;
        }

        m_hoverEffectTimerId = startTimer(HOVER_EFFECT_TIMEOUT);
    }
}

void AbstractTaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    stopWindowHoverEffect();

    QString backgroundPrefix;
    if (m_flags & TaskWantsAttention) {
        backgroundPrefix = "attention";
    } else if (m_flags & TaskIsMinimized) {
        backgroundPrefix = "minimized";
    } else if (m_flags & TaskHasFocus) {
        backgroundPrefix = "focus";
    } else {
        backgroundPrefix = "normal";
    }

    fadeBackground(backgroundPrefix, 150);
}

void AbstractTaskItem::stopWindowHoverEffect()
{
    if (m_hoverEffectTimerId) {
        killTimer(m_hoverEffectTimerId);
        m_hoverEffectTimerId = 0;
    }

    if (m_lastViewId && m_applet->highlightWindows()) {
        Plasma::WindowEffects::highlightWindows(m_lastViewId, QList<WId>());
    }
}

void AbstractTaskItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && boundingRect().contains(event->pos())) {
        activate();
    }
}

void AbstractTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    update();
}

void AbstractTaskItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug();
    if (QPoint(event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance()) {
        return;
    } //Wait a bit before starting drag

  /*  if((m_applet->taskSortOrder() != Tasks::NoSorting) && (m_applet->taskSortOrder() != Tasks::GroupSorting)){ //FIXME check somhow if drag is allowed
        return;
    }*/

    QByteArray data;
    data.resize(sizeof(AbstractTaskItem*));
    AbstractTaskItem *selfPtr = this;
    memcpy(data.data(), &selfPtr, sizeof(AbstractTaskItem*));

    QMimeData* mimeData = new QMimeData();
    setAdditionalMimeData(mimeData);

    if (mimeData->formats().isEmpty()) {
        delete mimeData;
        return;
    }

    QDrag *drag = new QDrag(event->widget());
    drag->setMimeData(mimeData);
    drag->setPixmap(icon().pixmap(20));
   // drag->setDragCursor( set the correct cursor //TODO
    drag->exec();
}

void AbstractTaskItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_activateTimerId) {
        killTimer(m_activateTimerId);
        m_activateTimerId = 0;
        if (!isActive()) {
            activate();
        }
    } else if (event->timerId() == m_updateGeometryTimerId) {
        killTimer(m_updateGeometryTimerId);
        m_updateGeometryTimerId = 0;
        m_firstGeometryUpdate = true;
        publishIconGeometry();
    } else if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        m_updateTimerId = 0;
        update();
    } else if (event->timerId() == m_attentionTimerId) {
        ++m_attentionTicks;
        if (m_attentionTicks > 6) {
            killTimer(m_attentionTimerId);
            m_attentionTimerId = 0;
            m_attentionTicks = 0;
        }

        if (m_attentionTicks % 2 == 0) {
            fadeBackground("attention", 200);
        } else {
            fadeBackground("normal", 250);
        }
    } else if (event->timerId() == m_hoverEffectTimerId) {
        killTimer(m_hoverEffectTimerId);
        m_hoverEffectTimerId = 0;
        if (!isUnderMouse()) {
            return;
        }

#ifdef Q_WS_X11
        QList<WId> windows;

        if (m_abstractItem && m_abstractItem->itemType() == TaskManager::GroupItemType) {
            TaskManager::TaskGroup *group = qobject_cast<TaskManager::TaskGroup *>(m_abstractItem);

            if (group) {
                TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(this);
                if (groupItem && groupItem->popupDialog()) {
                    kDebug() << "adding" << groupItem->popupDialog()->winId();
                    windows.append(groupItem->popupDialog()->winId());
                }

                foreach (AbstractGroupableItem *item, group->members()) {
                    if (item->itemType() == TaskManager::TaskItemType) {
                        TaskManager::TaskItem *taskItem = qobject_cast<TaskManager::TaskItem *>(item);
                        if (taskItem && taskItem->task()) {
                            windows.append(taskItem->task()->window());
                        }
                    } //TODO: if taskgroup, recurse through sub-groups?
                }
            }
        } else {
            WindowTaskItem *windowTaskItem = qobject_cast<WindowTaskItem *>(this);
            if (windowTaskItem && windowTaskItem->parent()) {
                TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(windowTaskItem->parent());
                if (groupItem && groupItem->popupDialog()) {
                    windows.append(groupItem->popupDialog()->winId());
                }
            }

            TaskManager::TaskItem *taskItem = qobject_cast<TaskManager::TaskItem *>(m_abstractItem);
            if (taskItem && taskItem->task()) {
                windows.append(taskItem->task()->window());
            }
        }

        stopWindowHoverEffect();
        QGraphicsView *view = m_applet->view();
        if (view && m_applet->highlightWindows()) {
            m_lastViewId = view->winId();
            Plasma::WindowEffects::highlightWindows(m_lastViewId, windows);
        }
#endif
    } else {
        QGraphicsWidget::timerEvent(event);
    }
}

void AbstractTaskItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    if (!m_abstractItem) {
        return;
    }
    //kDebug() << "painting" << (QObject*)this << text();
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_abstractItem->itemType() != TaskManager::LauncherItemType) { //Launchers have no frame
        // draw background
        drawBackground(painter, option, widget);
    }

    // draw icon and text
    drawTask(painter, option, widget);
}

void AbstractTaskItem::syncActiveRect()
{
    m_cachedShadow = QPixmap();
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();
    itemBackground->setElementPrefix("normal");

    qreal left, top, right, bottom;
    itemBackground->getMargins(left, top, right, bottom);

    itemBackground->setElementPrefix("focus");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    itemBackground->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    m_activeRect = QRectF(QPointF(0, 0), size());
    m_activeRect.adjust(left - activeLeft, top - activeTop,
                        -(right - activeRight), -(bottom - activeBottom));

    itemBackground->setElementPrefix(m_backgroundPrefix);

    // check to see if there is enough room!
    QFontMetrics fm(font());
    const int minimumWidth = left + 8 + IconTextSpacing + right;
    m_showText = (size().width() >= fm.width("M") * 6 + minimumWidth);
    queueUpdate();
}

void AbstractTaskItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    syncActiveRect();
    resizeBackground(event->newSize().toSize());
}

void AbstractTaskItem::resizeBackground(const QSize &size)
{
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    itemBackground->setElementPrefix("focus");
    m_applet->resizeItemBackground(size);
    itemBackground->setElementPrefix("normal");
    m_applet->resizeItemBackground(size);
    itemBackground->setElementPrefix("minimized");
    m_applet->resizeItemBackground(size);
    itemBackground->setElementPrefix("attention");
    m_applet->resizeItemBackground(size);
    itemBackground->setElementPrefix("hover");
    m_applet->resizeItemBackground(size);

    //restore the prefix
    itemBackground->setElementPrefix(m_backgroundPrefix);
}

void AbstractTaskItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    // Do not paint with invalid sizes, the happens when the layout is being initialized
    if (!option->rect.isValid()) {
        return;
    }

    /*FIXME -could be done more elegant with caching in tasks in a qhash <size,svg>.
    -do not use size() directly because this introduces the blackline syndrome.
    -This line is only needed when we have different items in the taskbar because of an expanded group for example. otherwise the resizing in the resizeEvent is sufficient
    */
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    if (~option->state & QStyle::State_Sunken && 
        (!m_backgroundFadeAnim || m_backgroundFadeAnim->state() != QAbstractAnimation::Running)) {
        itemBackground->setElementPrefix(m_backgroundPrefix);
        //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
        if (size().toSize() != itemBackground->frameSize()) {
            resizeBackground(size().toSize());
        }

        if (itemBackground->frameSize() == m_activeRect.size().toSize()) {
            itemBackground->paintFrame(painter, m_activeRect.topLeft());
        } else {
            itemBackground->paintFrame(painter);
        }
        //itemBackground->paintFrame(painter, backgroundPosition);
        return;
    }

    itemBackground->setElementPrefix(m_oldBackgroundPrefix);
    //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
    if (size().toSize() != itemBackground->frameSize()) {
        resizeBackground(size().toSize());
    }

    QPixmap oldBackground;

    if (option->state & QStyle::State_Sunken) {
        oldBackground = QPixmap(m_activeRect.size().toSize());
        oldBackground.fill(Qt::transparent);
        m_alpha = 0.4;
    } else {
        oldBackground = itemBackground->framePixmap();
    }

    itemBackground->setElementPrefix(m_backgroundPrefix);
    //since a single framesvg is shared between all tasks, we could have to resize it even if there wasn't a resizeevent
    if (size().toSize() != itemBackground->frameSize()) {
        resizeBackground(size().toSize());
    }

    QPixmap result = Plasma::PaintUtils::transition(oldBackground, itemBackground->framePixmap(), m_alpha);

    if (result.size() == m_activeRect.size().toSize()) {
        painter->drawPixmap(m_activeRect.topLeft(), result);
    } else {
        painter->drawPixmap(QPoint(0,0), result);
    }
}

void AbstractTaskItem::drawTask(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    QRectF bounds = boundingRect();

    if (m_abstractItem->itemType() != TaskManager::LauncherItemType) {
        bounds = bounds.adjusted(m_applet->itemLeftMargin(), m_applet->itemTopMargin(), -m_applet->itemRightMargin(), -m_applet->itemBottomMargin());
    } else {
        bounds = bounds.adjusted(5,5,-5,-5);
    }

    WindowTaskItem *window = qobject_cast<WindowTaskItem *>(this);
    QGraphicsWidget *busyWidget;
    busyWidget = window ? window->busyWidget() : 0;
    const QRectF iconR = iconRect(bounds);

    if (busyWidget) {
        busyWidget->setGeometry(iconR);
        busyWidget->show();
    } else {
        /*
        kDebug() << bool(option->state & QStyle::State_MouseOver) << m_backgroundFadeAnim <<
            (m_backgroundFadeAnim ? m_backgroundFadeAnim->state() : QAbstractAnimation::Stopped);*/
        const bool fadingBg = m_backgroundFadeAnim && m_backgroundFadeAnim->state() == QAbstractAnimation::Running;
        if ((!fadingBg && !(option->state & QStyle::State_MouseOver)) ||
            (m_oldBackgroundPrefix != "hover" && m_backgroundPrefix != "hover")) {
            // QIcon::paint does some alignment work and can lead to funny
            // things when icon().size() != iconR.toRect().size()
            QPixmap result = icon().pixmap(iconR.toRect().size());
            painter->drawPixmap(iconR.topLeft(), result);
        } else {
            KIconEffect *effect = KIconLoader::global()->iconEffect();
            QPixmap result = icon().pixmap(iconR.toRect().size());

            if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState)) {
                if (qFuzzyCompare(qreal(1.0), m_alpha)) {
                    result = effect->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
                } else {
                    result = Plasma::PaintUtils::transition(result,
                                        effect->apply(result, KIconLoader::Desktop,
                                        KIconLoader::ActiveState), m_backgroundPrefix != "hover" ? 1 - m_alpha : m_alpha);
                }
            }

            painter->drawPixmap(iconR.topLeft(), result);
        }
    }

    painter->setPen(QPen(textColor(), 1.0));

    if (m_abstractItem->itemType() != TaskManager::LauncherItemType) {
        if (m_showText) {
            QRect rect = textRect(bounds).toRect();
            if (rect.height() > 20) {
                rect.adjust(2, 2, -2, -2); // Create a text margin
            }
            QTextLayout layout;
            layout.setFont(KGlobalSettings::taskbarFont());
            layout.setTextOption(textOption());

            layoutText(layout, text(), rect.size());
            drawTextLayout(painter, layout, rect);
        }

        TaskGroupItem *groupItem = qobject_cast<TaskGroupItem *>(this);
        if (groupItem) {
            QFont font(KGlobalSettings::smallestReadableFont());
            QFontMetrics fm(font);
            QRectF rect(expanderRect(bounds));

            Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

            if (itemBackground->hasElement(expanderElement())) {
                QSizeF arrowSize(itemBackground->elementSize(expanderElement()));
                QRectF arrowRect(rect.center()-QPointF(arrowSize.width()/2, arrowSize.height()+fm.xHeight()/2), arrowSize);
                itemBackground->paint(painter, arrowRect, expanderElement());

                painter->setFont(font);
                rect.setTop(arrowRect.bottom());
                painter->drawText(rect, Qt::AlignHCenter|Qt::AlignTop, QString::number(groupItem->count()));
            } else {
                painter->setFont(font);
                painter->drawText(rect, Qt::AlignCenter, QString::number(groupItem->count()));
            }
        }
    }
}

QTextOption AbstractTaskItem::textOption() const
{
    Qt::LayoutDirection direction = QApplication::layoutDirection();
    Qt::Alignment alignment = QStyle::visualAlignment(direction, Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption option;
    option.setTextDirection(direction);
    option.setAlignment(alignment);

    return option;
}

QSize AbstractTaskItem::layoutText(QTextLayout &layout, const QString &text,
                                   const QSize &constraints) const
{
    QFontMetrics metrics(layout.font());
    int leading     = metrics.leading();
    int height      = 0;
    int maxWidth    = constraints.width();
    int widthUsed   = 0;
    int lineSpacing = metrics.lineSpacing();
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();
    while ((line = layout.createLine()).isValid()) {
        height += leading;

        // Make the last line that will fit infinitely long.
        // drawTextLayout() will handle this by fading the line out
        // if it won't fit in the constraints.
        if (height + 2 * lineSpacing > constraints.height()) {
            line.setPosition(QPoint(0, height));
            break;
        }

        line.setLineWidth(maxWidth);
        line.setPosition(QPoint(0, height));

        height += int(line.height());
        widthUsed = int(qMax(qreal(widthUsed), line.naturalTextWidth()));
    }
    layout.endLayout();

    return QSize(widthUsed, height);
}

void AbstractTaskItem::drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect)
{
    if (rect.width() < 1 || rect.height() < 1) {
        return;
    }

    QPixmap pixmap(rect.size());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(painter->pen());

    // Create the alpha gradient for the fade out effect
    QLinearGradient alphaGradient(0, 0, 1, 0);
    alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    if (layout.textOption().textDirection() == Qt::LeftToRight)
    {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));
    } else
    {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 0));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 255));
    }

    QFontMetrics fm(layout.font());
    int textHeight = layout.lineCount() * fm.lineSpacing();

    QPointF position(0, (rect.height() - textHeight) / 2 + (fm.tightBoundingRect("M").height() - fm.xHeight())/2);
    QList<QRect> fadeRects;
    int fadeWidth = 30;

    // Draw each line in the layout
    for (int i = 0; i < layout.lineCount(); i++)
    {
        QTextLine line = layout.lineAt(i);
        line.draw(&p, position);

        // Add a fade out rect to the list if the line is too long
        if (line.naturalTextWidth() > rect.width())
        {
            int x = int(qMin(line.naturalTextWidth(), (qreal)pixmap.width())) - fadeWidth;
            int y = int(line.position().y() + position.y());
            QRect r = QStyle::visualRect(layout.textOption().textDirection(), pixmap.rect(),
                                         QRect(x, y, fadeWidth, int(line.height())));
            fadeRects.append(r);
        }
    }

    // Reduce the alpha in each fade out rect using the alpha gradient
    if (!fadeRects.isEmpty())
    {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        foreach (const QRect &rect, fadeRects) {
            p.fillRect(rect, alphaGradient);
        }
    }

    p.end();


    QColor shadowColor;
    if (qGray(textColor().rgb()) > 192) {
        shadowColor = Qt::black;
    } else {
        shadowColor = Qt::white;
    }

    if (m_cachedShadow.isNull()) {
        QImage shadow = pixmap.toImage();
        Plasma::PaintUtils::shadowBlur(shadow, 2, shadowColor);
        m_cachedShadow = QPixmap(shadow.size());
        m_cachedShadow.fill(Qt::transparent);
        QPainter buffPainter(&m_cachedShadow);
        buffPainter.drawImage(QPoint(0,0), shadow);
    }

    if (shadowColor == Qt::white) {
        painter->drawPixmap(rect.topLeft(), m_cachedShadow);
    } else {
        painter->drawPixmap(rect.topLeft() + QPoint(1,2), m_cachedShadow);
    }
    painter->drawPixmap(rect.topLeft(), pixmap);
}


qreal AbstractTaskItem::backgroundFadeAlpha() const
{
    return m_alpha;
}

void AbstractTaskItem::setBackgroundFadeAlpha(qreal progress)
{
    m_alpha = progress;
    update();
}

bool AbstractTaskItem::shouldIgnoreDragEvent(QGraphicsSceneDragDropEvent *event)
{
   if (event->mimeData()->hasFormat(TaskManager::Task::mimetype()) ||
       event->mimeData()->hasFormat(TaskManager::Task::groupMimetype())) {
        return true;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        // we want to check if we have executables; if so, then we treat it as a possible
        // drop for a launcher
        const KUrl::List uris = KUrl::List::fromMimeData(event->mimeData());
        if (!uris.isEmpty()) {
            foreach (const QUrl &uri, uris) {
                KUrl url(uri);
                if (url.isLocalFile()) {
                    const QString path = url.toLocalFile();
                    QFileInfo info(path);
                    if (info.isDir() || !info.isExecutable()) {
                        return false;
                        break;
                    }
                }
            }

            return true;
        }
    }

    return false;
}

void AbstractTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (shouldIgnoreDragEvent(event)) {
        event->ignore();
        return;
    }

    event->accept();

    if (!m_activateTimerId) {
        m_activateTimerId = startTimer(500);
    }
}

void AbstractTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    // restart the timer so that activate() is only called after the mouse
    // stops moving
    if (m_activateTimerId) {
        killTimer(m_activateTimerId);
        m_activateTimerId = startTimer(500);
    }
}

void AbstractTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    if (m_activateTimerId) {
        killTimer(m_activateTimerId);
        m_activateTimerId = 0;
    }
}

QRect AbstractTaskItem::iconGeometry() const
{
    if (!scene() || !boundingRect().isValid()) {
        return QRect();
    }

    QGraphicsView *parentView = 0;
    QGraphicsView *possibleParentView = 0;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            if (view->isActiveWindow()) {
                parentView = view;
                break;
            } else {
                possibleParentView = view;
            }
        }
    }

    if (!parentView) {
        parentView = possibleParentView;

        if (!parentView) {
            return QRect();
        }
    }

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    return rect;
}

void AbstractTaskItem::publishIconGeometry() const
{
}

void AbstractTaskItem::publishIconGeometry(const QRect &rect) const
{
    Q_UNUSED(rect)
}

void AbstractTaskItem::setAnimationPos(const QPointF &pos)
{
    m_layoutAnimationLock = true;
    setPos(pos);
    m_layoutAnimationLock = false;
}

QPointF AbstractTaskItem::animationPos() const
{
    return pos();
}

void AbstractTaskItem::setGeometry(const QRectF& geometry)
{
    if (geometry == QGraphicsWidget::geometry()) {
        return;
    }

    QPointF oldPos = pos();

    if (m_lastGeometryUpdate.elapsed() < 500) {
        if (m_updateGeometryTimerId) {
            killTimer(m_updateGeometryTimerId);
            m_updateGeometryTimerId = 0;
        }

        m_updateGeometryTimerId = startTimer(500 - m_lastGeometryUpdate.elapsed());
    } else {
        publishIconGeometry();
        m_lastGeometryUpdate.restart();
    }

    //TODO:remove when we will have proper animated layouts
    if (m_firstGeometryUpdate && !m_layoutAnimationLock) {
        QRectF animStartGeom(oldPos, geometry.size());
        QGraphicsWidget::setGeometry(animStartGeom);

        if (m_layoutAnimation->state() == QAbstractAnimation::Running) {
            m_layoutAnimation->stop();
        }

        m_layoutAnimation->setEndValue(geometry.topLeft());
        m_layoutAnimation->start();
    } else {
        QGraphicsWidget::setGeometry(geometry);
    }
}

QRectF AbstractTaskItem::iconRect(const QRectF &b)
{
    QRectF bounds(b);
    const int right = bounds.right();

    if (m_showText) {
        //leave enough space for the text. useful in vertical panel
        bounds.setWidth(qMax(bounds.width() / 3, qMin(minimumSize().height(), bounds.width())));
    }

    //restore right position if the layout is RTL
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        bounds.moveRight(right);
    }

    QSize iconSize = icon().actualSize(bounds.size().toSize());

    if (iconSize.width() == iconSize.height()) {
        if (iconSize.width() > KIconLoader::SizeSmall && iconSize.width() < KIconLoader::SizeSmallMedium) {
            iconSize = QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        } else if (iconSize.width() > KIconLoader::SizeSmallMedium && iconSize.width() < KIconLoader::SizeMedium) {
            iconSize = QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        } else if (iconSize.width() > KIconLoader::SizeMedium && iconSize.width() < KIconLoader::SizeLarge) {
            iconSize = QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
        }
    }

    if (iconSize != m_lastIconSize) {
        m_cachedShadow = QPixmap();
    }
    m_lastIconSize = iconSize;
    return QStyle::alignedRect(QApplication::layoutDirection(),
                               (m_showText ? Qt::AlignLeft : Qt::AlignCenter) | Qt::AlignVCenter,
                               iconSize, bounds.toRect());
}

QRectF AbstractTaskItem::expanderRect(const QRectF &bounds)
{
    const TaskGroupItem *groupItem = qobject_cast<const TaskGroupItem *>(this);
    if (!groupItem) {
        return QRectF();
    }

    QFontMetrics fm(KGlobalSettings::smallestReadableFont());
    Plasma::FrameSvg *itemBackground = m_applet->itemBackground();

    QSize expanderSize(qMax(fm.width(QString::number(groupItem->count())),
                       itemBackground->elementSize(expanderElement()).width()),
                       size().height());

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                               expanderSize, bounds.toRect());
}

QRectF AbstractTaskItem::textRect(const QRectF &bounds)
{
    QSize size(bounds.size().toSize());
    QRectF effectiveBounds(bounds);

    size.rwidth() -= int(iconRect(bounds).width()) + qMax(0, IconTextSpacing - 2);
    if (!isWindowItem()) {
        size.rwidth() -= int(expanderRect(bounds).width()) + qMax(0, IconTextSpacing - 2);

        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            effectiveBounds.setLeft(expanderRect(bounds).right());
        } else {
            effectiveBounds.setRight(expanderRect(bounds).left());
        }
    }

    return QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                                     size, effectiveBounds.toRect());
}

QColor AbstractTaskItem::textColor() const
{
    QColor color;
    qreal bias;
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    if ((m_oldBackgroundPrefix == "attention" || m_backgroundPrefix == "attention") &&
         m_applet->itemBackground()->hasElement("hint-attention-button-color")) {
        bool animatingBg = m_backgroundFadeAnim && m_backgroundFadeAnim->state() == QAbstractAnimation::Running;
        if (animatingBg) {
            if (m_oldBackgroundPrefix == "attention") {
                bias = 1 - m_alpha;
            } else {
                bias = m_alpha;
            }

            color = KColorUtils::mix(theme->color(Plasma::Theme::TextColor),
                                     theme->color(Plasma::Theme::ButtonTextColor), bias);
        } else if (m_backgroundPrefix != "attention") {
                color = theme->color(Plasma::Theme::TextColor);
        } else {
            color = theme->color(Plasma::Theme::ButtonTextColor);
        }
    } else {
        color = theme->color(Plasma::Theme::TextColor);
    }

    if (m_flags & TaskIsMinimized) {
        color.setAlphaF(0.85);
    }

    return color;
}

QString AbstractTaskItem::expanderElement() const
{
    switch (m_applet->location()) {
    case Plasma::TopEdge:
        return "group-expander-top";
    case Plasma::RightEdge:
        return "group-expander-right";
    case Plasma::LeftEdge:
        return "group-expander-left";
    case Plasma::BottomEdge:
    default:
        return "group-expander-bottom";
    }
}


bool AbstractTaskItem::isGroupMember(const TaskGroupItem *group) const
{
    if (!m_abstractItem || !group) {
        kDebug() <<"no task";
        return false;
    }

    return m_abstractItem->isGroupMember(group->group());

}

bool AbstractTaskItem::isGrouped() const
{
    if (!m_abstractItem) {
        kDebug() <<"no item";
        return false;
    }

    return m_abstractItem->isGrouped();
}

TaskGroupItem * AbstractTaskItem::parentGroup() const
{
    TaskGroupItem *group = qobject_cast<TaskGroupItem*>(parentWidget());

    //lucky case: directly in a group
    if (group) {
        return group;
    }

    //in a popup or a popup's popup?
    QObject *candidate = parentWidget();

    while (candidate) {
        group = qobject_cast<TaskGroupItem*>(candidate);
        candidate = candidate->parent();
        if (group) {
            return group;
        }
    }

    return 0;
}

TaskManager::AbstractGroupableItem * AbstractTaskItem::abstractItem()
{
    return m_abstractItem;
}

#include "abstracttaskitem.moc"
