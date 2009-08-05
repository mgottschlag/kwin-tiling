#include "managewidgets.h"

ManageWidgetsPushButton::ManageWidgetsPushButton(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    init();
}

ManageWidgetsPushButton::~ManageWidgetsPushButton(){
}


void ManageWidgetsPushButton::init()
{
    m_button = new KPushButton();
    m_button->setFlat(true);
    m_button->setAttribute(Qt::WA_NoSystemBackground);
    m_buttonProxy = new QGraphicsProxyWidget();
    m_buttonProxy->setWidget(m_button);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_buttonProxy);
    layout->setAlignment(m_buttonProxy, Qt::AlignVCenter);
    setLayout(layout);
}

KPushButton *ManageWidgetsPushButton::button()
{
    return m_button;
}

KMenu *ManageWidgetsPushButton::buttonMenu()
{
    return m_buttonMenu;
}


QGraphicsProxyWidget *ManageWidgetsPushButton::buttonProxy()
{
    return m_buttonProxy;
}
