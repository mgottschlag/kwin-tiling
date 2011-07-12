/***************************************************************************
 *   Copyright 2007 by Enrico Ros <enrico.ros@gmail.com>                   *
 *   Copyright 2007 by Riccardo Iaconelli <ruphy@kde.org>                  *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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


#include "resultitem.h"

#include <math.h>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QGraphicsItemAnimation>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <QTimeLine>
#include <QTimer>

#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/PaintUtils>
#include <Plasma/ToolButton>
#include <Plasma/Plasma>
#include <Plasma/RunnerManager>

//#define NO_GROW_ANIM

void shadowBlur(QImage &image, int radius, const QColor &color);

ResultItem::ResultItem(const SharedResultData *sharedData, const Plasma::QueryMatch &match, Plasma::RunnerManager *runnerManager, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_match(0),
      m_configButton(0),
      m_highlight(0),
      m_index(-1),
      m_configWidget(0),
      m_actionsWidget(0),
      m_actionsLayout(0),
      m_runnerManager(runnerManager),
      m_sharedData(sharedData),
      m_mouseHovered(false),
      m_mimeDataFailed(false)
{
    m_highlightCheckTimer.setInterval(0);
    m_highlightCheckTimer.setSingleShot(true);
    connect(&m_highlightCheckTimer, SIGNAL(timeout()), this, SLOT(checkHighlighting()));
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFocusPolicy(Qt::TabFocus);
    setCacheMode(DeviceCoordinateCache);
    setZValue(0);

    m_highlightAnim = new QPropertyAnimation(this, "highlightState", this);
    m_highlightAnim->setStartValue(0);
    m_highlightAnim->setEndValue(1);
    m_highlightAnim->setDuration(50);
    m_highlightAnim->setEasingCurve(QEasingCurve::OutCubic);
    setMatch(match);
}

ResultItem::~ResultItem()
{
}

QGraphicsWidget* ResultItem::arrangeTabOrder(QGraphicsWidget* last)
{
    QGraphicsWidget *sceneWidget = static_cast<QGraphicsWidget*>(parent());
    sceneWidget->setTabOrder(last, this);
    QGraphicsWidget *currentWidget = this;

    if (m_configButton) {
        sceneWidget->setTabOrder(this, m_configButton);
        currentWidget = m_configButton;
        if (m_configWidget) {
            sceneWidget->setTabOrder(m_configButton, m_configWidget);
            currentWidget = m_configWidget;
        }
    }
    if (m_actionsWidget) {
        for (int i = 0; i < m_actionsLayout->count(); ++i) {
            QGraphicsWidget *button = static_cast<QGraphicsWidget*>(m_actionsLayout->itemAt(i));
            sceneWidget->setTabOrder(currentWidget, button);
            currentWidget = button;
        }
    }

    return currentWidget;
}

bool ResultItem::isValid() const
{
    return m_match.isValid() || m_match.type() == Plasma::QueryMatch::InformationalMatch;
}

void ResultItem::setMatch(const Plasma::QueryMatch &match)
{
    m_mimeDataFailed = false;
    m_match = match;
    m_icon = KIcon(match.icon());

    if (m_configWidget) {
        if (scene()) {
            scene()->removeItem(m_configWidget);
        }

        delete m_configWidget;
        m_configWidget = 0;
    }

    if (m_actionsWidget) {
        if (scene()) {
            scene()->removeItem(m_actionsWidget);
        }

        delete m_actionsWidget;
        m_actionsWidget = 0;
    }

    //kDebug() << match.hasConfigurationInterface();
    if (match.hasConfigurationInterface()) {
        if (!m_configButton) {
            m_configButton = new Plasma::ToolButton(this);
            m_configButton->setIcon(KIcon(QLatin1String( "configure" )));
            m_configButton->show();
            m_configButton->resize(m_configButton->effectiveSizeHint(Qt::MinimumSize,
                                                                     QSize(KIconLoader::SizeSmall,
                                                                           KIconLoader::SizeSmall)));
            connect(m_configButton, SIGNAL(clicked()), this, SLOT(showConfig()));
            m_configButton->installEventFilter(this);
        }
    } else if (m_configButton) {
        if (scene()) {
            scene()->removeItem(m_configButton);
        }

        delete m_configButton;
        m_configButton = 0;
    }

    setupActions();
    calculateSize();

    if (!m_match.isValid() && isSelected() && scene()) {
        scene()->clearSelection();
    }

    update();
}

void ResultItem::setupActions()
{
    //kDebug();
    QList<QAction*> actionList = m_runnerManager->actionsForMatch(m_match);

    if (!actionList.isEmpty()) {
        m_actionsWidget = new QGraphicsWidget(this);
        m_actionsLayout = new QGraphicsLinearLayout(Qt::Horizontal, m_actionsWidget);

        foreach (QAction* action, actionList) {
            Plasma::ToolButton * actionButton = new Plasma::ToolButton(m_actionsWidget);
            actionButton->setFlag(QGraphicsItem::ItemIsFocusable);
            actionButton->setAction(action);
            actionButton->show();
            actionButton->resize(actionButton->effectiveSizeHint(Qt::MinimumSize,
                                                        QSize(KIconLoader::SizeSmall,
                                                              KIconLoader::SizeSmall)));
            m_actionsLayout->addItem(actionButton);
            connect(actionButton, SIGNAL(clicked()), this , SLOT(actionClicked()));
            actionButton->installEventFilter(this);
        }
        m_actionsWidget->show();
    }
}

void ResultItem::actionClicked()
{
    Plasma::ToolButton * actionButton = static_cast<Plasma::ToolButton*>(sender());
    m_match.setSelectedAction(actionButton->action());
    emit activated(this);
}

bool ResultItem::eventFilter(QObject *obj, QEvent *event)
{
    Plasma::ToolButton* actionButton = static_cast<Plasma::ToolButton*>(obj);

    if (actionButton) {
        if (event->type() == QEvent::GraphicsSceneHoverEnter) {
            if (scene()) {
                scene()->setFocusItem(actionButton);
            }
        } else if (event->type() == QEvent::FocusIn) {
            focusInEvent(static_cast<QFocusEvent*>(event));
            actionButton->setAutoRaise(false);
        } else if (event->type() == QEvent::GraphicsSceneHoverLeave || event->type() == QEvent::FocusOut) {
            actionButton->setAutoRaise(true);
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                if (actionButton->action()) {
                    m_match.setSelectedAction(actionButton->action());
                    emit activated(this);
                } else {
                    showConfig();
                }
                return true;
            }
        }
    }

    return false;
}

QString ResultItem::id() const
{
    return m_match.id();
}

bool ResultItem::compare(const ResultItem *one, const ResultItem *other)
{
    return other->m_match < one->m_match;
}

bool ResultItem::operator<(const ResultItem &other) const
{
    return m_match < other.m_match;
}

QString ResultItem::name() const
{
    return m_match.text();
}

QString ResultItem::description() const
{
    if (!scene()) {
        return QString();
    }

    Plasma::ToolButton* actionButton = qobject_cast<Plasma::ToolButton*>(static_cast<QGraphicsWidget*>(scene()->focusItem()));

    //if a button is focused and it  belongs to the item
    if (actionButton && actionButton->parentWidget() == m_actionsWidget) {
        return actionButton->text();
    }

    return m_match.subtext();
}

QString ResultItem::data() const
{
    return m_match.data().toString();
}

QIcon ResultItem::icon() const
{
    return m_icon;
}

Plasma::QueryMatch::Type ResultItem::group() const
{
    return m_match.type();
}

bool ResultItem::isQueryPrototype() const
{
    //FIXME: pretty lame way to decide if this is a query prototype
    return m_match.runner() == 0;
}

qreal ResultItem::priority() const
{
    // TODO, need to fator in more things than just this
    return m_match.relevance();
}

void ResultItem::setIndex(int index)
{
    if (m_index == index) {
        return;
    }

    m_index = qMax(-1, index);
}

int ResultItem::index() const
{
    return m_index;
}

void ResultItem::run(Plasma::RunnerManager *manager)
{
    manager->run(m_match);
}

void ResultItem::drawIcon(QPainter *painter, const QRect &iRect, const QPixmap &p)
{
    //QPixmap p = m_icon.pixmap(iconSize, QIcon::Active);
    QRect r = p.rect();
    r.moveCenter(iRect.center());
    painter->drawPixmap(r, p);
}

void ResultItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    if (!m_match.isValid() && m_match.type() != Plasma::QueryMatch::InformationalMatch) {
        return;
    }

    bool oldClipping = painter->hasClipping();
    painter->setClipping(false);

    QSize iconSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    QRect iRect = QStyle::alignedRect(option->direction, Qt::AlignLeft, iconSize, contentsRect().toRect());

    painter->setRenderHint(QPainter::Antialiasing);

    if (qFuzzyCompare(m_highlight + 1, 1)) {
        drawIcon(painter, iRect, m_icon.pixmap(iconSize, QIcon::Disabled));
    } else if (qFuzzyCompare(m_highlight, qreal(1.0))) {
        drawIcon(painter, iRect, m_icon.pixmap(iconSize, QIcon::Active));
    } else {
        painter->setOpacity(painter->opacity() * (1 - m_highlight));
        drawIcon(painter, iRect, m_icon.pixmap(iconSize, QIcon::Disabled));
        painter->setOpacity(m_highlight);
        drawIcon(painter, iRect, m_icon.pixmap(iconSize, QIcon::Active));
        painter->setOpacity(1);
    }

    QRect textRect(iRect.topLeft() + QPoint(iconSize.width() + TEXT_MARGIN, 0),
                   contentsRect().size().toSize() - QSize(iRect.width(), 0));
    if (option->direction == Qt::RightToLeft) {
        textRect.moveRight(iRect.left() - TEXT_MARGIN);
    }

    // Draw the text on a pixmap
    const QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    const int width = option->fontMetrics.width(name());
    QPixmap pixmap(textRect.size());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(textColor);
    //TODO: add subtext, make bold, etc...
    p.drawText(pixmap.rect(), Qt::AlignLeft | Qt::TextWordWrap, name());
    QFont italics = p.font();
    QFontMetrics italicMetrics(italics);
    int fontHeight = italicMetrics.boundingRect(pixmap.rect(), Qt::AlignLeft | Qt::TextWordWrap, name()).height();
    italics.setItalic(true);
    p.setFont(italics);
    p.drawText(pixmap.rect().adjusted(0, fontHeight, 0, 0), Qt::AlignLeft | Qt::TextWordWrap, description());

    // Fade the pixmap out at the end
    if (width > pixmap.width()) {
        if (m_fadeout.isNull() || m_fadeout.height() != pixmap.height()) {
            QLinearGradient g(0, 0, 20, 0);
            g.setColorAt(0, layoutDirection() == Qt::LeftToRight ? Qt::white : Qt::transparent);
            g.setColorAt(1, layoutDirection() == Qt::LeftToRight ? Qt::transparent : Qt::white);
            m_fadeout = QPixmap(20, textRect.height());
            m_fadeout.fill(Qt::transparent);
            QPainter p(&m_fadeout);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(m_fadeout.rect(), g);
        }
        const QRect r = QStyle::alignedRect(layoutDirection(), Qt::AlignRight, m_fadeout.size(), pixmap.rect());
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(r.topLeft(), m_fadeout);
    }
    p.end();

    // Draw a drop shadow if we have a bright text color
    if (qGray(textColor.rgb()) > 192) {
        const int blur = 2;
        const QPoint offset(1, 1);

        QImage shadow(pixmap.size() + QSize(blur * 2, blur * 2), QImage::Format_ARGB32_Premultiplied);
        p.begin(&shadow);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(shadow.rect(), Qt::transparent);
        p.drawPixmap(blur, blur, pixmap);
        p.end();

        Plasma::PaintUtils::shadowBlur(shadow, blur, Qt::black);

        // Draw the shadow
        painter->drawImage(textRect.topLeft() - QPoint(blur, blur) + offset, shadow);
    }

    // Draw the text
    painter->drawPixmap(textRect.topLeft(), pixmap);
    painter->setClipping(oldClipping);
}

void ResultItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    //kDebug() << "in on" << m_match.text() << m_sharedData->processHoverEvents;
    if (!m_sharedData->processHoverEvents || !m_match.isValid()) {
        return;
    }

    QGraphicsItem::hoverEnterEvent(e);
    setFocus(Qt::MouseFocusReason);
}

void ResultItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (geometry().contains(event->scenePos())) {
        emit activated(this);
    }
}

void ResultItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_mimeDataFailed &&
        event->buttons() == Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).manhattanLength() >= KGlobalSettings::dndEventDelay()) {
        QMimeData *mime = m_runnerManager->mimeDataForMatch(m_match);
        //kDebug() << mime << m_match.text() << m_match.id() << m_match.data();
        if (mime) {
            QDrag *drag = new QDrag(event->widget());
            drag->setMimeData(mime);
            drag->exec();
        }

        m_mimeDataFailed = !mime;
    }
}

bool ResultItem::mouseHovered() const
{
    return m_mouseHovered;
}

void ResultItem::focusInEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusInEvent(event);
    //kDebug() << hasFocus();
    setZValue(1);

    m_mouseHovered = (event->reason() == Qt::MouseFocusReason);

    if (scene()) {
        scene()->clearSelection();
    }

    setSelected(true);
    emit ensureVisibility(this);
}

void ResultItem::highlight(bool yes)
{
    if (yes) {
        if (m_highlight < 1) {
            m_highlightAnim->setDirection(QAbstractAnimation::Forward);
            m_highlightAnim->start();
        }
    } else if (m_highlight > 0) {
        m_highlightAnim->setDirection(QAbstractAnimation::Backward);
        m_highlightAnim->start();
    }
}

qreal ResultItem::highlightState() const
{
    return m_highlight;
}

void ResultItem::setHighlightState(qreal highlight)
{
    m_highlight = highlight;
    update();
}

void ResultItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit activated(this);
    } else {
        QGraphicsWidget::keyPressEvent(event);
    }
}

QVariant ResultItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged) {
        calculateSize();
    } else if (change == QGraphicsItem::ItemSelectedHasChanged && !isSelected()) {
        m_highlightCheckTimer.start();
    }

    return QGraphicsWidget::itemChange(change, value);
}

void ResultItem::checkHighlighting()
{
    highlight(isSelected());
}

void ResultItem::resizeEvent(QGraphicsSceneResizeEvent *)
{
    emit sizeChanged(this);
}

void ResultItem::changeEvent(QEvent *event)
{
    QGraphicsWidget::changeEvent(event);

    if (event->type() == QEvent::ContentsRectChange) {
        calculateSize();
    }
}

void ResultItem::showConfig()
{
    if (m_configWidget) {
        if (scene()) {
            scene()->removeItem(m_configWidget);
        }

        delete m_configWidget;
        m_configWidget = 0;
    } else {
        QWidget *w = new QWidget;
        m_match.createConfigurationInterface(w);
        w->setAttribute(Qt::WA_NoSystemBackground);
        m_configWidget = new QGraphicsProxyWidget(this);
        m_configWidget->setWidget(w);
        m_configWidget->show();
        QGraphicsWidget* sceneWidget = parentWidget();
        sceneWidget->setTabOrder(m_configButton, m_configWidget);
    }

    calculateSize();
    update();
}

void ResultItem::calculateSize()
{
    if (scene()) {
        calculateSize(scene()->width());
    }
}

void ResultItem::calculateSize(int sceneWidth)
{
    QRect textBounds(contentsRect().toRect());

    textBounds.setWidth(sceneWidth);

    QString text = name();

    if (!description().isEmpty()) {
        text.append(QLatin1Char( '\n' )).append(description());
    }

    const QFontMetrics fm(font());
    const int maxHeight = fm.height() * 4;
    const int minHeight = KIconLoader::SizeMedium;

    textBounds.adjust(minHeight + TEXT_MARGIN, 0, 0, 0);

    if (maxHeight > textBounds.height()) {
        textBounds.setHeight(maxHeight);
    }

    int height = fm.boundingRect(textBounds, Qt::AlignLeft | Qt::TextWordWrap, text).height();
    //kDebug() << (QObject*)this << text << fm.boundingRect(textBounds, Qt::AlignLeft | Qt::TextWordWrap, text);
    //kDebug() << fm.height() << maxHeight << textBounds << height << minHeight << qMax(height, minHeight);
    int innerHeight = qMax(height, minHeight);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QSize newSize(sceneWidth, innerHeight + top + bottom);
    //kDebug() << innerHeight << geometry().size();

    if (m_configButton) {
        QSizeF s = m_configButton->size();

        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            m_configButton->setPos(left, newSize.height() - s.height() - bottom);
        } else {
            m_configButton->setPos(newSize.width() - s.width() - right,
                                   newSize.height() - s.height() - bottom);
        }
    }

    if (m_configWidget) {
        m_configWidget->setMaximumWidth(newSize.width());
        m_configWidget->adjustSize();
        newSize.setHeight(newSize.height() + m_configWidget->size().height());
        m_configWidget->setPos((newSize.width() - m_configWidget->size().width()) / 2,
                               newSize.height() - m_configWidget->size().height() - bottom);
    }

    if (m_actionsWidget) {
        m_actionsWidget->setMaximumWidth(newSize.width()/2);
        m_actionsWidget->adjustSize();
        QSizeF s = m_actionsWidget->size();

        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            m_actionsWidget->setPos(left, newSize.height() - s.height() - bottom);
        } else {
            m_actionsWidget->setPos(newSize.width() - s.width() - right,
                                   newSize.height() - s.height() - bottom);
        }
    }

    resize(newSize);
}

#include "resultitem.moc"

