#ifndef APPLETTOOLTIP_H
#define APPLETTOOLTIP_H

#include "plasmaappletitemmodel_p.h"
#include "appleticon.h"

#include <QtCore>
#include <QtGui>

#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/tabbar.h>
#include <plasma/dialog.h>

class AppletInfoWidget : public QGraphicsWidget {

    Q_OBJECT

    public:
        AppletInfoWidget(QGraphicsItem *parent = 0, PlasmaAppletItem *appletItem = 0);
        ~AppletInfoWidget();

        void init();
        void setAppletItem(PlasmaAppletItem *appletItem);

    public Q_SLOTS:
        void updateInfo();
        void onInfoButtonClick();

    Q_SIGNALS:
        void infoButtonClicked(const QString &apluginName);

    private:
        PlasmaAppletItem * m_appletItem;
        QGraphicsLinearLayout * m_mainLayout;

        Plasma::IconWidget * m_iconWidget;
        Plasma::Label  * m_nameLabel;
        Plasma::TabBar * m_tabs;

        Plasma::Label   * m_aboutLabel;
        Plasma::Label   * m_actionsLabel;
        Plasma::Label   * m_detailsLabel;

        // Plasma::IconWidget * m_infoButton;
};

class AppletToolTipWidget : public Plasma::Dialog {

    Q_OBJECT

    public:
        explicit AppletToolTipWidget(QWidget *parent = 0, AppletIconWidget *applet = 0);
        virtual ~AppletToolTipWidget();

        void setAppletIconWidget(AppletIconWidget *applet);
        void updateContent();
        AppletIconWidget *appletIconWidget();

    Q_SIGNALS:
        void enter();
        void leave();
        void infoButtonClicked(const QString &apluginName);

    protected:
        void enterEvent(QEvent *event);
        void leaveEvent(QEvent *event);

    private:
        AppletIconWidget *m_applet;
        AppletInfoWidget *m_widget;
};

#endif //APPLETTOOLTIP_H
