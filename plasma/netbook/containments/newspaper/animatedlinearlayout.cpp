#include "animatedlinearlayout.h"
#include "../common/proxylayout.h"

#include <QGraphicsWidget>
#include <QDebug>

AnimatedLinearLayout::AnimatedLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent)
    : QGraphicsLinearLayout(orientation, parent)
{
}

AnimatedLinearLayout::AnimatedLinearLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLinearLayout(parent)
{
}

void AnimatedLinearLayout::addItem(QGraphicsLayoutItem *item)
{
    if(!item->isLayout()) {
        ProxyLayout *proxyItem = new ProxyLayout(static_cast<QGraphicsWidget*>(item));
        QGraphicsLinearLayout::addItem(proxyItem);
    } else
        QGraphicsLinearLayout::addItem(item);
}

void AnimatedLinearLayout::removeAt(int index)
{
    QGraphicsLayoutItem *layoutItem = QGraphicsLinearLayout::itemAt(index);

    QGraphicsLinearLayout::removeAt(index);
    ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
    if(layout) {
        delete layout;
    }
}
