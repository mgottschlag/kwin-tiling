#ifndef APPLETICON_H
#define APPLETICON_H

#include "plasmaappletitemmodel_p.h"

#include <QtCore>
#include <QtGui>

#include <plasma/framesvg.h>
#include <plasma/widgets/iconwidget.h>

class AppletIconWidget : public Plasma::IconWidget
{
    Q_OBJECT

    public:
        explicit AppletIconWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        virtual ~AppletIconWidget();

        void setAppletItem(PlasmaAppletItem *appletIcon);
        void setSelected(bool selected);
        PlasmaAppletItem *appletItem();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    public Q_SLOTS:
        void updateApplet(PlasmaAppletItem *newAppletItem);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    Q_SIGNALS:
        void hoverEnter(AppletIconWidget *applet);
        void hoverLeave(AppletIconWidget *applet);
        void selected(AppletIconWidget *applet);
        void doubleClicked(AppletIconWidget *applet);

    private:
        PlasmaAppletItem *m_appletItem;
        bool m_selected;
        bool m_hovered;
        Plasma::FrameSvg *m_selectedBackgroundSvg;
        bool m_showingTooltip;
};

#endif //APPLETICON_H
