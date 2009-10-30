#include "animatedgridlayout.h"
#include "../common/proxylayout.h"

#include <QGraphicsWidget>
#include <KDebug>

AnimatedGridLayout::AnimatedGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsGridLayout(parent)
{
}

void AnimatedGridLayout::addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment)
{
    if(!item->isLayout()) {
        ProxyLayout *proxyItem = new ProxyLayout(static_cast<QGraphicsWidget*>(item));
        QGraphicsGridLayout::addItem(proxyItem, row, column, alignment);
    } else {
        QGraphicsGridLayout::addItem(item, row, column, alignment);
    }
}

QGraphicsLayoutItem *AnimatedGridLayout::itemAt(int index) const
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(index);

    if (layoutItem) {
        ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
        if (layout) {
            return layout->widget();
        }
    }

    return layoutItem;
}

QGraphicsLayoutItem *AnimatedGridLayout::itemAt(int row, int column) const
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(row, column);
    if(layoutItem) {
        ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
        if(layout) {
            return layout->widget();
        }
    }

    return layoutItem;
}


void AnimatedGridLayout::removeAt(int index)
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(index);

    ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
    if(layout) {
        delete layout;
    }
    QGraphicsGridLayout::removeAt(index);
}
