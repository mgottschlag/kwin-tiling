#ifndef ANIMATEDLINEARLAYOUT_H
#define ANIMATEDLINEARLAYOUT_H

#include <QGraphicsGridLayout>
#include <QObject>

class AnimatedGridLayout :public QGraphicsGridLayout
{
public:
    AnimatedGridLayout(QGraphicsLayoutItem *parent = 0);

    void addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment = 0);
    QGraphicsLayoutItem *itemAt(int index) const;
    QGraphicsLayoutItem *itemAt(int row, int column) const;
    void removeAt(int index);
};

#endif
