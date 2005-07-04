/* 
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDRPASSIVEPOPUP_H__
#define __RANDRPASSIVEPOPUP_H__

#include <kpassivepopup.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <X11/Xlib.h>

class KRandrPassivePopup
    : public KPassivePopup
    {
    Q_OBJECT
    public:
	static KRandrPassivePopup *message( const QString &caption, const QString &text,
	    const QPixmap &icon, QWidget *parent, const char *name=0, int timeout = -1 );
    protected:
	virtual bool eventFilter( QObject* o, QEvent* e );
	virtual bool x11Event( XEvent* e );
    private slots:
	void slotPositionSelf();
    private:
        KRandrPassivePopup( QWidget *parent=0, const char *name=0, WFlags f=0 );
	void startWatchingWidget( QWidget* w );
	QValueList< QWidget* > watched_widgets;
	QValueList< Window > watched_windows;
	QTimer update_timer;
    };

#endif
