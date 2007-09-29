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

#include <kglobal.h>
#include <klocale.h>

#include "kxkbwidget.h"
#include "kxkbcore.h"
#include "kxkb_applet.h"

#include "kxkb_applet.moc"


extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("kxkb");
        int actions = Plasma::Preferences | Plasma::About | Plasma::Help;
        return new KxkbApplet(configFile, Plasma::Normal, actions, parent);
    }
}


KxkbApplet::KxkbApplet(const QString& configFile, Plasma::Type type,
                 int actions, QWidget *parent, Qt::WFlags f)
    : KPanelApplet(configFile, type, actions, parent, f)
{
    move( 0, 0 );
    kxkbWidget = new KxkbLabel( this );
	KxkbCore* kxkbCore = new KxkbCore(kxkbWidget);
	kxkbCore->newInstance();
    //setCustomMenu(widget->history()->popup());
    //centerWidget();
    //kxkbWidget->show();
}

KxkbApplet::~KxkbApplet()
{
}

int KxkbApplet::widthForHeight(int height) const
{
	return 32;//kxkbWidget->width();
}

int KxkbApplet::heightForWidth(int width) const
{
	return 32;//kxkbWidget->height();
}
