/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kiconloader.h>

#include "trashapplet.h"

extern "C"
{
	KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString& configFile)
	{
		KGlobal::locale()->insertCatalog("trashapplet");
		return new TrashApplet(configFile, Plasma::Normal,
			Plasma::About, parent);
	}
}

TrashApplet::TrashApplet(const QString& configFile, Plasma::Type type, int actions, QWidget *parent)
	: KPanelApplet(configFile, type, actions, parent), mButton(0)
{
	mButton = new TrashButton(this);

	if (!parent)
        {
            setBackgroundRole( QPalette::NoRole );
            setForegroundRole( QPalette::NoRole );
	}
	mButton->setPanelPosition(position());

	setAcceptDrops(true);

	mpDirLister = new KDirLister();

	connect( mpDirLister, SIGNAL( clear() ),
	         this, SLOT( slotClear() ) );
	connect( mpDirLister, SIGNAL( completed() ),
	         this, SLOT( slotCompleted() ) );
	connect( mpDirLister, SIGNAL( deleteItem( KFileItem * ) ),
	         this, SLOT( slotDeleteItem( KFileItem * ) ) );

	mpDirLister->openURL(KUrl("trash:/"));
}

TrashApplet::~TrashApplet()
{
	// disconnect the dir lister before quitting so as not to crash
	// on kicker exit
	disconnect( mpDirLister, SIGNAL( clear() ),
	            this, SLOT( slotClear() ) );
	delete mpDirLister;
	KGlobal::locale()->removeCatalog("trashapplet");
}

void TrashApplet::about()
{
	KAboutData data("trashapplet",
	                I18N_NOOP("Trash Applet"),
	                "1.0",
	                I18N_NOOP("\"trash:/\" ioslave frontend applet"),
	                KAboutData::License_GPL_V2,
	                "(c) 2004, Kevin Ottens");

	data.addAuthor("Kevin \'ervin\' Ottens",
	               I18N_NOOP("Maintainer"),
	               "ervin ipsquad net",
	               "http://ervin.ipsquad.net");

	KAboutApplication dialog(&data);
	dialog.exec();
}

int TrashApplet::widthForHeight( int height ) const
{
	if ( !mButton )
	{
		return height;
	}

	return mButton->widthForHeight( height );
}

int TrashApplet::heightForWidth( int width ) const
{
	if ( !mButton )
	{
		return width;
	}

	return mButton->heightForWidth( width );
}

void TrashApplet::resizeEvent( QResizeEvent * )
{
	if (!mButton)
	{
		return;
	}

	int size = 1;

	size = std::max( size,
	                 orientation() == Qt::Vertical ?
	                 	mButton->heightForWidth( width() ) :
	                 	mButton->widthForHeight( height() ) );


	if(orientation() == Qt::Vertical)
	{
		mButton->resize( width(), size );
	}
	else
	{
		mButton->resize( size, height() );
	}
}

void TrashApplet::slotClear()
{
	kDebug()<<"MediaApplet::slotClear"<<endl;

	mButton->setItemCount(0);
}

void TrashApplet::slotCompleted()
{
	mCount = mpDirLister->items(KDirLister::AllItems).count();
	mButton->setItemCount( mCount );
}

void TrashApplet::slotDeleteItem(KFileItem *)
{
	mCount--;
	mButton->setItemCount( mCount );
}


void TrashApplet::positionChange(Plasma::Position p)
{
	mButton->setPanelPosition(p);
}


#include "trashapplet.moc"
