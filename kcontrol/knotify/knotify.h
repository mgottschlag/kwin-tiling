/* 

    $Id$

    Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                  1998 Bernd Wuebben <wuebben@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Log$
    Revision 1.8  2000/03/14 05:18:18  pbrown
    merged bell settings into here. Other cleanups.

    Revision 1.7  2000/01/08 18:59:26  kulow
    some sparcCC fixes

    Revision 1.6  1999/11/15 14:29:45  hoelzer
    Ported to the new module concept.

    Revision 1.5  1999/07/13 23:49:50  pbrown
    KDND is dead, long live Xdnd.

    Revision 1.4  1999/03/12 18:40:58  dfaure
    Squashed more ksprintf and did some more Qt2.0 porting

    Revision 1.3  1999/03/01 23:24:12  kulow
    CVS_SILENT ported to Qt 2.0

    Revision 1.2.4.1  1999/02/22 22:19:43  kulow
    CVS_SILENT replaced old qt header names with new ones

    Revision 1.2  1998/03/08 08:01:33  wuebben
    Bernd: implemented support for all sound events


*/  


#ifndef _KNOTIFY_H
#define _KNOTIFY_H

#include "kcmodule.h"

#include <qstringlist.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#include <mediatool.h>
#include <kaudio.h>
#include <knuminput.h>

class KNotifyWidget : public KCModule
{
Q_OBJECT

public:
	KNotifyWidget(QWidget *parent, const char *name);
	virtual ~KNotifyWidget();

	void defaults();


private slots:
	void changed();

};

#endif
