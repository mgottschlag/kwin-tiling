#ifndef PROXYLAYOUT_H
#define PROXYLAYOUT_H

#include <QGraphicsLayout>
#include <QObject>

class ProxyLayoutPrivate;

class ProxyLayout :public QGraphicsLayoutItem
{
public:
    ProxyLayout(QGraphicsWidget *widget, QGraphicsLayoutItem *parent = 0);
    ~ProxyLayout();

    QGraphicsLayoutItem *widget();
    void setGeometry(const QRectF &rect);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint=QSizeF()) const;

private:
    ProxyLayoutPrivate  * const d;
};

#endif
