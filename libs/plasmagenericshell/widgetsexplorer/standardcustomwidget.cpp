#include "standardcustomwidget.h"

StandardCustomWidget::StandardCustomWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags)
        : QGraphicsWidget(parent, wFlags)
{
    m_svg = new Plasma::FrameSvg(this);
    m_svg->setImagePath("widgets/background");
}

 void StandardCustomWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_svg->resizeFrame(contentsRect().size());
     m_svg->paintFrame(painter, contentsRect().topLeft());
 }

 void StandardCustomWidget::setBackgroundSvg(QString svg)
 {
    m_svg->setImagePath(svg);
 }





