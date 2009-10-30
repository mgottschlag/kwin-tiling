#ifndef ANIMATEDLINEARLAYOUT_H
#define ANIMATEDLINEARLAYOUT_H

#include <QGraphicsLinearLayout>

class AnimatedLinearLayout : public QGraphicsLinearLayout
{
public:
    AnimatedLinearLayout(QGraphicsLayoutItem *parent = 0);
    AnimatedLinearLayout(Qt::Orientation, QGraphicsLayoutItem *parent = 0);

    void addItem(QGraphicsLayoutItem *item);
    void removeAt(int index);
};

#endif
