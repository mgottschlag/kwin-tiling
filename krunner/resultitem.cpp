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

#include <QtCore/QTimeLine>
#include <QtCore/QDebug>
#include <KDebug>
#include <QtCore/QtGlobal>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsItemAnimation>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsScene>

#include <kicon.h>

#include <plasma/plasma.h>

class ResultItem::Private {
public:
    Private(ResultItem *item)
        : q(item),
          match(0),
          tempTransp(1.0),
          highlight(false),
          index(-1),
          rowStride(6),
          updateId(0),
          highlightTimerId(0),
          animation(0),
          needsMoving(false)
    {
    }

    ~Private()
    {
        delete animation;
    }

    static ResultItemSignaller * signaller()
    {
        if (!s_signaller) {
            s_signaller = new ResultItemSignaller;
        }

        return s_signaller;
    }

    QPointF pos();
    void appear();
    void move(bool randomStart = true);
    void init();
    //void animationComplete();

    static ResultItemSignaller *s_signaller;
    static int s_removingCount;
    static ResultItem *s_defaultItem;

    ResultItem * q;
    Plasma::QueryMatch match;
    // static description
    QIcon       icon;
    // dyn params
    QColor      tempColor;
    qreal       tempTransp;
    int highlight;
    int index;
    int rowStride;
    int updateId;
    int highlightTimerId;
    QGraphicsItemAnimation *animation;
    bool isFavorite : 1;
    bool needsMoving : 1;
    bool selected : 1;
};

int ResultItem::Private::s_removingCount = 0;
ResultItemSignaller* ResultItem::Private::s_signaller = 0;
ResultItem *ResultItem::Private::s_defaultItem = 0;

QPointF ResultItem::Private::pos()
{
    int row = (index / rowStride) * ResultItem::BOUNDING_SIZE;
    int col = (index % rowStride) * ResultItem::BOUNDING_SIZE; //(w / rowStride);
    //kDebug() << col << row << "for" << index;
    return QPointF(col, row);
}

void ResultItem::Private::appear()
{
    if (animation) {
        delete animation;
        animation = 0;
    }

    QPointF p(pos());
    qreal halfway = ResultItem::BOUNDING_SIZE * 0.5;
    qreal mostway = ResultItem::BOUNDING_SIZE * 0.1;

    q->setPos(pos());
    animation = new QGraphicsItemAnimation();
    animation->setItem(q);
    animation->setScaleAt(0.0, 0.0, 0.0);
    animation->setScaleAt(0.5, 0.1, 1.0);
    animation->setScaleAt(1.0, 1.0, 1.0);
    animation->setPosAt(0.0, p + QPointF(halfway, halfway));
    animation->setPosAt(0.5, p + QPointF(mostway, 0));
    animation->setPosAt(1.0, p);
    QTimeLine * timer = new QTimeLine(100);
    animation->setTimeLine(timer);

    timer->start();
    QTimer::singleShot(50, q, SLOT(becomeVisible()));
    connect(timer, SIGNAL(finished()), q, SLOT(animationComplete()));
}

void ResultItem::Private::move(bool randomStart)
{
    //qDebug() << "moving to" << index << rowStride;
    if (animation) {
        delete animation;
        animation = 0;
    }

    QTimeLine *timer = new QTimeLine();

    if (randomStart) {
        timer->setDuration(250);
    } else {
        //TODO: port to phase
        timer->setDuration(150);
    }
    timer->setCurveShape(QTimeLine::EaseOutCurve);

    QGraphicsLayoutItem *parent =  q->parentLayoutItem();
    QRect contentsRect = parent ? parent->contentsRect().toRect() : q->scene()->sceneRect().toRect();
    int h = contentsRect.height();
    int w = contentsRect.width();

    QGraphicsItemAnimation * animation = new QGraphicsItemAnimation(q);
    animation->setItem(q);
    animation->setTimeLine(timer);

    if (randomStart) {
       animation->setPosAt(0.0, QPointF(qrand() % (w - ResultItem::BOUNDING_SIZE),
                                        qrand() % (h - ResultItem::BOUNDING_SIZE)));
    }

    animation->setPosAt(1.0, pos());
    QObject::connect(timer, SIGNAL(finished()), q, SLOT(animationComplete()));
    timer->start();
}

//void ResultItem::Private::animationComplete()
void ResultItem::animationComplete()
{
    delete d->animation;
    d->animation = 0;
    resetTransform();
}

ResultItem::ResultItem(const Plasma::QueryMatch &match, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new Private(this))
{
    setMatch(match);
    d->init();
    connect(Private::signaller(), SIGNAL(animate()), this, SLOT(animate()));
}

void ResultItem::Private::init()
{
    //QTimer * timer = new QTimer(q);
    //connect(timer, SIGNAL(timeout()), q, SLOT(slotTestTransp()));
    //timer->start(50);

    q->setFlag(QGraphicsItem::ItemIsFocusable);
    q->setFlag(QGraphicsItem::ItemIsSelectable);
    q->setAcceptHoverEvents(true);
    q->setFocusPolicy(Qt::TabFocus);
    q->resize(ITEM_SIZE, ITEM_SIZE);
}

ResultItem::~ResultItem()
{
    if (Private::s_defaultItem == this) {
        Private::s_defaultItem = 0;
    }

    --Private::s_removingCount;

    if (Private::s_removingCount < 1) {
        Private::s_removingCount = 0;
        Private::signaller()->startAnimations();
    }
    delete d;
}

void ResultItem::setMatch(const Plasma::QueryMatch &match)
{
    d->match = match;
    d->icon = KIcon(match.icon());

    int hue = 0;
    switch (match.type()) {
        case Plasma::QueryMatch::CompletionMatch:
            hue = 10; // reddish
            break;
        case Plasma::QueryMatch::InformationalMatch:
        case Plasma::QueryMatch::HelperMatch:
            hue = 110; // green
            break;
        case Plasma::QueryMatch::ExactMatch:
            hue = 60; // gold
            break;
        case Plasma::QueryMatch::PossibleMatch:
        default:
            hue = 40; // browny
            break;
            break;
    }

//     d->tempColor = QColor::fromHsv(hue, 0, 150);
    d->tempColor = QColor(61, 61, 61);
}

QString ResultItem::id() const
{
    return d->match.id();
}

int ResultItem::updateId() const
{
    return d->updateId;
}

void ResultItem::setUpdateId(int id)
{
    d->updateId = id;
}

QString ResultItem::name() const
{
    return d->match.text();
}

QString ResultItem::description() const
{
    return d->match.subtext();
}

QString ResultItem::data() const
{
    return d->match.data().toString();
}

QIcon ResultItem::icon() const
{
    return d->icon;
}

Plasma::QueryMatch::Type ResultItem::group() const
{
    return d->match.type();
}

uint ResultItem::priority() const
{
    // TODO, need to fator in more things than just this
    return d->match.relevance();
}

bool ResultItem::isFavorite() const
{
    return d->isFavorite;
}

void ResultItem::setIsDefault(bool isDefault)
{
    if (isDefault) {
        if (Private::s_defaultItem != this) {
            Private::s_defaultItem = this;
            update();
        }
    } else if (Private::s_defaultItem == this) {
        Private::s_defaultItem = 0;
        update();
    }
}

bool ResultItem::isDefault() const
{
    return Private::s_defaultItem == this;
}

void ResultItem::setSelected(bool selected)
{
    d->selected = selected;
    update();
}

bool ResultItem::selected() const
{
    return d->selected;
}

void ResultItem::setIndex(int index)
{
    if (d->index == index) {
        return;
    }

    bool first = d->index == -1;
    d->index = index;
    d->needsMoving = false;

    if (index < 0) {
        index = -1;
        return;
    }

    //kDebug() << index << first;
    if (first) {
        d->appear();
    } else if (d->s_removingCount) {
        d->needsMoving = true;
    } else {
        d->move(false);
    }
}

void ResultItem::animate()
{
    //kDebug() << "can animate now" << d->needsMoving;
    if (d->needsMoving) {
        d->needsMoving = false;
        d->move(false);
    }
}

void ResultItem::becomeVisible()
{
    show();
}

int ResultItem::index() const
{
    return d->index;
}

void ResultItem::setRowStride(int stride)
{
    if (d->rowStride == stride) {
        return;
    }

    d->rowStride = stride;
    if (d->index != -1) {
        d->move(false);
    }
}

int ResultItem::rowStride() const
{
    return d->rowStride;
}

void ResultItem::remove()
{
    if (d->animation) {
        delete d->animation;
        d->animation = 0;
    }

    d->needsMoving = false;
    d->animation = new QGraphicsItemAnimation();
    d->animation->setItem(this);
    d->animation->setScaleAt(0.0, 1.0, 1.0);
    d->animation->setScaleAt(0.5, 0.1, 1.0);
    d->animation->setScaleAt(1.0, 0.0, 0.0);
    d->animation->setPosAt(0.0, d->pos() + QPointF(0.0, 0.0));
    d->animation->setPosAt(0.5, d->pos() + QPointF(32.0*0.9, 0.0));
    d->animation->setPosAt(1.0, d->pos() + QPointF(32.0, 32.0));
    QTimeLine * timer = new QTimeLine(150);
    d->animation->setTimeLine(timer);
    ++Private::s_removingCount;

    connect(timer, SIGNAL(finished()), this, SLOT(deleteLater()));
    timer->start();
}

void ResultItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    bool oldClipping = painter->hasClipping();
    painter->setClipping(false);

    QRect iRect = boundingRect().toRect().adjusted(PADDING, PADDING, -PADDING, -PADDING);
    bool mouseOver = option->state & QStyle::State_MouseOver || hasFocus();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->save();

    // Draw icon frame
    // TODO: Make me SVG themable!
    painter->setBrush(d->tempColor);//QColor(51, 51, 51));
    painter->setPen(QPen(Qt::transparent, 0));
    painter->drawPath(Plasma::roundedRectangle(boundingRect(), 6));

    painter->restore();

    painter->save();

        /*
        The following implements blinking Blinking
    if (mouseOver || d->tempTransp >= 1.5) {
        painter->setOpacity(1.0);
    } else {
        qreal transp = d->tempTransp - int(d->tempTransp);
        //qDebug() << "transparency of" << transp << "from" << d->tempTransp;
        if (transp > 0.5) {
            painter->setOpacity(2.0 - (2.0 * transp));
        } else {
            painter->setOpacity(2.0 * transp);
        }

        if (!d->highlightTimerId) {
            d->highlightTimerId = startTimer(40);
        }
    }
        */

    if (!mouseOver && d->tempTransp < 0.9) {
        painter->setOpacity(d->tempTransp);

        if (!d->highlightTimerId) {
            d->highlightTimerId = startTimer(40);
        }
    }

    bool drawMixed = false;

    //TODO: add bool selected()
    if (mouseOver || isDefault()) {
        if (d->highlight > 2) {
            painter->drawPixmap(PADDING, PADDING, d->icon.pixmap(iRect.size(), QIcon::Active));
        } else {
            drawMixed = true;
            ++d->highlight;
            if (!d->highlightTimerId) {
                d->highlightTimerId = startTimer(40);
            }
        }
    } else if (d->highlight > 0) {
        drawMixed = true;
        --d->highlight;
        if (!d->highlightTimerId) {
            d->highlightTimerId = startTimer(40);
        }
    } else {
        painter->drawPixmap(PADDING, PADDING, d->icon.pixmap(iRect.size(), QIcon::Disabled));
    }

    if (drawMixed) {
        qreal factor = .2;

        if (d->highlight == 1) {
            factor = .4;
        } else if (d->highlight == 2) {
            factor = .6;
        } else if (d->highlight > 2) {
            factor = .8;
        }

        qreal activeOpacity = painter->opacity() * factor;

        painter->setOpacity(painter->opacity() * (1 - factor));
        painter->drawPixmap(PADDING, PADDING, d->icon.pixmap(iRect.size(), QIcon::Disabled));
        painter->setOpacity(activeOpacity);
        painter->drawPixmap(PADDING, PADDING, d->icon.pixmap(iRect.size(), QIcon::Active));
    }
    painter->restore();

    painter->save();
    // Draw hover/selection rects
    // TODO: Make me themable with the SVG!
    if (selected()) {
        painter->translate(0.5, 0.5);
        painter->setBrush(Qt::transparent);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawPath(Plasma::roundedRectangle(boundingRect(), 6));
    }
    painter->restore();


    int textHeight = option->fontMetrics.height();
// qWarning() << textHeight;
    QRect textRect = iRect;
    textRect.setTop(textRect.bottom() - textHeight);
//     painter->drawText(textRect, Qt::AlignCenter, m_description);
//     textRect.translate(0, -textHeight);
    painter->drawText(textRect, Qt::AlignCenter, name());
    painter->setPen(Qt::white);
    painter->drawText(textRect.translated(-1, -1), Qt::AlignCenter, name());

    painter->setClipping(oldClipping);
}

void ResultItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    QGraphicsItem::hoverLeaveEvent(e);
    update();
    emit hoverLeave(this);
}

void ResultItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    QGraphicsItem::hoverEnterEvent(e);
    update();
    emit hoverEnter(this);
}

void ResultItem::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)

    d->tempTransp += 0.1;
    killTimer(d->highlightTimerId);
    d->highlightTimerId = 0;

    update();
}

void ResultItem::slotTestTransp()
{
    d->tempTransp += 0.02;
    if (d->tempTransp >= 1.0)
        d->tempTransp -= 1.0;
    update();
}

void ResultItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit activated(this);
}

void ResultItem::focusInEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusInEvent(event);
    emit hoverEnter(this);
    update();
}

void ResultItem::focusOutEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusOutEvent(event);
    emit hoverLeave(this);
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

#include "resultitem.moc"

