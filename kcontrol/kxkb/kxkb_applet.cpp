/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QWidget>
#include <QSizeF>
#include <QMenu>

#include <kglobal.h>
#include <klocale.h>

#include <plasma/widgets/boxlayout.h>

#include "kxkbwidget.h"
#include "kxkbcore.h"
#include "kxkb_applet.h"


// Plasma widget

KxkbPlasmaWidget::KxkbPlasmaWidget(QGraphicsItem* parent, int controlType) :
    KxkbWidget(controlType),
    m_displayMode(ICON)
{
//        if( controlType == ICON )
    m_indicatorWidget = new Plasma::Icon(parent);
    m_indicatorWidget->setIconSize(32,32);
    connect(m_indicatorWidget, SIGNAL(clicked()), this, SIGNAL(iconToggled()));
//        else
//            m_indicatorWidget = new Plasma::PushButton(parent);
    m_menu = new QMenu(NULL); // TODO: proper parent
}


// Plasma applet

KxkbApplet::KxkbApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    m_kxkbCore = new KxkbCore(KxkbCore::KXKB_COMPONENT);
    if( m_kxkbCore->newInstance() < 0 ) {
        setFailedToLaunch(true);
    }
    else {
        new Plasma::HBoxLayout(this);
        setDrawStandardBackground(true);
        KxkbWidget* kxkbWidget = new KxkbPlasmaWidget(this, KxkbWidget::MENU_FULL);
        m_kxkbCore->setWidget(kxkbWidget);
//        m_kxkbWidget = kxkbWidget;
    }
    //setCustomMenu(widget->history()->popup());
    //centerWidget();
    //kxkbWidget->show();
}

KxkbApplet::~KxkbApplet()
{
    if (failedToLaunch()) {
        delete m_kxkbCore;
        // Do some cleanup here
    } else {
        // Save settings
    }
}

QSizeF KxkbApplet::contentSizeHint() const
{
//    return QSizeF(m_kxkbCore->size());
    return QSizeF(32,32);
}
/*
void KxkbApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                    const QRect& contentsRect)
{
//    Q_UNUSED(option);
    
    ((KxkbPlasmaWidget*)m_kxkbWidget)->widget()->paint(painter, option, NULL);
}
*/

#include "kxkb_applet.moc"
