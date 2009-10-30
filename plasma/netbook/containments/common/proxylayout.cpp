#include "proxylayout.h"

#include <QPropertyAnimation>
#include <QGraphicsWidget>

#include <QWeakPointer>

#include <KDebug>

class ProxyLayoutPrivate
{
public:
    QWeakPointer<QGraphicsWidget> widget;
    QWeakPointer<QPropertyAnimation> animation;
};

ProxyLayout::ProxyLayout(QGraphicsWidget *widget, QGraphicsLayoutItem *parent)
    : QGraphicsLayoutItem(parent, false), d(new ProxyLayoutPrivate)
{

    setOwnedByLayout(true);

    d->widget = widget;
    setGraphicsItem(widget);

    QPropertyAnimation *animation = new QPropertyAnimation(widget, "geometry", widget);
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->setDuration(1000);

    d->animation = animation;
}

ProxyLayout::~ProxyLayout()
{
    if(d->widget.data())
        d->widget.clear();
}

void ProxyLayout::setGeometry(const QRectF &rect)
{
    if(!d->animation.data()) {
        return;
    }

    QPropertyAnimation *animation = d->animation.data();

    if(animation->state() == QAbstractAnimation::Running)
        animation->stop();

    QGraphicsWidget *widget = d->widget.data();

    if(!widget) {
        QGraphicsLayoutItem::setGeometry(rect);
        return;
    }

    animation->setEndValue(rect);
    animation->start();
}

QSizeF ProxyLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    QGraphicsWidget *widget = d->widget.data();
    QSizeF currentWidgetSize = widget->effectiveSizeHint(which, constraint);

    return QSizeF( left + right + currentWidgetSize.width(), right + bottom + currentWidgetSize.height());
}

QGraphicsLayoutItem *ProxyLayout::widget()
{
    return d->widget.data();
}
