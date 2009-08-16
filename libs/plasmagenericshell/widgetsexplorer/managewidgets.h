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
