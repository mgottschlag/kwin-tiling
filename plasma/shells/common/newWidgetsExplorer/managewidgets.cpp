/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


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
