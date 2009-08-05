#ifndef MANAGEWIDGETS_H
#define MANAGEWIDGETS_H

#include <QtCore>
#include <QtGui>

#include <kmenu.h>
#include <kpushbutton.h>

class ManageWidgetsPushButton : public QGraphicsWidget
{

    Q_OBJECT

    public:
        explicit ManageWidgetsPushButton(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~ManageWidgetsPushButton();

        void init();
        KPushButton *button();
        QGraphicsProxyWidget *buttonProxy();
        KMenu *buttonMenu();

    private:
        KPushButton *m_button;
        QGraphicsProxyWidget *m_buttonProxy;
        KMenu *m_buttonMenu;
};

#endif //MANAGEWIDGETS_H
