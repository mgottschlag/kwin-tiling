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

#include <kglobal.h>
#include <klocale.h>

#include "kxkbwidget.h"
#include "kxkbcore.h"
#include "kxkb_applet.h"

#include "kxkb_applet.moc"


KxkbApplet::KxkbApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
//    move( 0, 0 );
    m_kxkbCore = new KxkbCore(NULL, KxkbCore::KXKB_COMPONENT, KxkbWidget::MENU_FULL, KxkbWidget::WIDGET_LABEL);
    m_kxkbCore->newInstance();

    setDrawStandardBackground(true);
//    connect(m_systemTrayWidget, SIGNAL(sizeChanged()), SLOT(updateLayout()));
    //m_kxkbWidget->show();
    //setCustomMenu(widget->history()->popup());
    //centerWidget();
    //kxkbWidget->show();
}

KxkbApplet::~KxkbApplet()
{
    if (failedToLaunch()) {
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

void KxkbApplet::paintInterface(QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                                    const QRect& contentsRect)
{
}
