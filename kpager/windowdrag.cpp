/*************************************************************************

    windowdrag.cpp  - The windowDrag object, used to drag windows across
       	                desktops

    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez <larrosa@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

*************************************************************************/

#include <config.h>

#include <stdio.h>

#include "windowdrag.h"

PagerWindowDrag::PagerWindowDrag(WId w,int deltax,int deltay, int origdesk,QWidget *parent)
    : Q3StoredDrag("application/x-kpager",parent,"windowdrag")
{
    QString tmp;
    tmp.sprintf("%d %d %d %d", static_cast<int>(w), deltax, deltay, origdesk);
    QByteArray data = tmp.toLatin1();

    setEncodedData(data);
}

PagerWindowDrag::~PagerWindowDrag()
{
}

bool PagerWindowDrag::canDecode (QDragMoveEvent *e)
{
    return e->provides("application/x-kpager");
}

bool PagerWindowDrag::decode( QDropEvent *e, WId &w,int &deltax,int &deltay,int &origdesk)
{
    QByteArray data=e->encodedData("application/x-kpager");
    if (data.size())
	{
	    char *tmp=data.data();
	    sscanf(tmp,"%lu %d %d %d", &w, &deltax, &deltay, &origdesk);
	    e->accept();
	    return true;
	}
    return false;
}
