#ifndef STANDARDCUSTOMWIDGET_H
#define STANDARDCUSTOMWIDGET_H

#include<QtGui>
#include <QGraphicsSvgItem>
#include <plasma/widgets/iconwidget.h>
#include <plasma/framesvg.h>

class StandardCustomWidget : public QGraphicsWidget
{

public:
    StandardCustomWidget(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);

    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    void setBackgroundSvg(QString svg);

private:
    Plasma::FrameSvg *m_svg;

};

#endif // STANDARDCUSTOMWIDGET_H
