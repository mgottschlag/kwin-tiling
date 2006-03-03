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

#include <QMouseEvent>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kiconloader.h>

#include "mediaapplet.h"

#include "preferencesdialog.h"

extern "C"
{
	KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString configFile)
	{
		KGlobal::locale()->insertCatalog("mediaapplet");
		return new MediaApplet(configFile, Plasma::Normal,
			Plasma::About | Plasma::Preferences,
			parent, "mediaapplet");
	}
}

MediaApplet::MediaApplet(const QString& configFile, Plasma::Type type, int actions, QWidget *parent, const char *name)
	: KPanelApplet(configFile, type, actions, parent, name),
	mButtonSizeSum(0)
{
	if (!parent)
            setBackgroundMode(Qt::X11ParentRelative);

	setAcceptDrops(true);

	loadConfig();

	mpDirLister = new KDirLister();

	connect( mpDirLister, SIGNAL( clear() ),
	         this, SLOT( slotClear() ) );
	connect( mpDirLister, SIGNAL( started(const KUrl&) ),
	         this, SLOT( slotStarted(const KUrl&) ) );
	connect( mpDirLister, SIGNAL( completed() ),
	         this, SLOT( slotCompleted() ) );
	connect( mpDirLister, SIGNAL( newItems( const KFileItemList & ) ),
	         this, SLOT( slotNewItems( const KFileItemList & ) ) );
	connect( mpDirLister, SIGNAL( deleteItem( KFileItem * ) ),
	         this, SLOT( slotDeleteItem( KFileItem * ) ) );
	connect( mpDirLister, SIGNAL( refreshItems( const KFileItemList & ) ),
	         this, SLOT( slotRefreshItems( const KFileItemList & ) ) );

	reloadList();
}

MediaApplet::~MediaApplet()
{
	delete mpDirLister;

	if (!mButtonList.isEmpty())
	{
            mButtonList.clear();
	}

        KGlobal::locale()->removeCatalog("mediaapplet");
}

void MediaApplet::about()
{
	KAboutData data("mediaapplet",
	                I18N_NOOP("Media Applet"),
	                "1.0",
	                I18N_NOOP("\"media:/\" ioslave frontend applet"),
	                KAboutData::License_GPL_V2,
	                "(c) 2004, Kevin Ottens");

	data.addAuthor("Kevin \'ervin\' Ottens",
	               I18N_NOOP("Maintainer"),
	               "ervin ipsquad net",
	               "http://ervin.ipsquad.net");

	data.addCredit("Joseph Wenninger",
	               I18N_NOOP("Good mentor, patient and helpful. Thanks for all!"),
	               "jowenn@kde.org");

	KAboutApplication dialog(&data);
	dialog.exec();
}

void MediaApplet::preferences()
{
	PreferencesDialog dialog(mMedia);

	dialog.setExcludedMediumTypes(mExcludedTypesList);
	dialog.setExcludedMedia(mExcludedList);

	if(dialog.exec())
	{
		mExcludedTypesList = dialog.excludedMediumTypes();
		mExcludedList = dialog.excludedMedia();
		saveConfig();
		reloadList();
	}
}

int MediaApplet::widthForHeight( int /*height*/ ) const
{
	return mButtonSizeSum;
}

int MediaApplet::heightForWidth( int /*width*/ ) const
{
	return mButtonSizeSum;
}

void MediaApplet::resizeEvent( QResizeEvent *)
{
	arrangeButtons();
}

void MediaApplet::arrangeButtons()
{
	int button_size = 1;
	int x_offset = 0;
	int y_offset = 0;

	// Determine upper bound for the button size
	MediumButtonList::iterator it;
	MediumButtonList::iterator end = mButtonList.end();
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;

		button_size = std::max(button_size,
		                       orientation() == Qt::Vertical ?
				           button->heightForWidth(width()) :
					   button->widthForHeight(height()) );
//		                           button->widthForHeight(height()) :
//		                           button->heightForWidth(width()) );
	}

	int kicker_size;
	if (orientation() == Qt::Vertical)
	{
		kicker_size = width();
	}
	else
	{
		kicker_size = height();
	}

	unsigned int max_packed_buttons = kicker_size / button_size;
	// Center icons if we only have one column/row
	if (mButtonList.count() < int(max_packed_buttons))
	{
		max_packed_buttons = qMax(1, mButtonList.count());
	}

	int padded_button_size = kicker_size / max_packed_buttons;
	mButtonSizeSum = 0;
	unsigned int pack_count = 0;

	// Arrange the buttons. If kicker is more than twice as high/wide
	// as the maximum preferred size of an icon, we put several icons
	// in one column/row
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;

		button->move(x_offset, y_offset);
		button->setPanelPosition(position());

		if(pack_count == 0)
		{
			mButtonSizeSum += button_size;
		}

		++pack_count;

		if(orientation() == Qt::Vertical)
                {
			if (pack_count < max_packed_buttons)
			{
				x_offset += padded_button_size;
			}
			else
			{
				x_offset = 0;
				y_offset += button_size;
				pack_count = 0;
			}

			button->resize(padded_button_size, button_size);
		}
		else
		{
			if (pack_count < max_packed_buttons)
			{
				y_offset += padded_button_size;
                        }
			else
			{
				y_offset = 0;
				x_offset += button_size;
				pack_count = 0;
			}

			button->resize(button_size, padded_button_size);
		}
	}

	updateGeometry();
	emit updateLayout();
}

void MediaApplet::slotClear()
{
	kDebug()<<"MediaApplet::slotClear"<<endl;

	if (!mButtonList.isEmpty())
	{
            mButtonList.clear();
	}

	arrangeButtons();
}

void MediaApplet::slotStarted(const KUrl &/*url*/)
{

}

void MediaApplet::slotCompleted()
{
	mMedia = mpDirLister->items(KDirLister::AllItems);
}

void MediaApplet::slotNewItems(const KFileItemList &entries)
{
	kDebug()<<"MediaApplet::slotNewItems"<<endl;

	foreach(KFileItem *item, entries)
	{
		kDebug() << "item: " << item->url() << endl;

		bool found = false;
		MediumButtonList::iterator it2;
		MediumButtonList::iterator end = mButtonList.end();
		for ( it2 = mButtonList.begin(); it2 != end; ++it2 )
		{
			MediumButton *button = *it2;

			if(button->fileItem().url()==item->url())
			{
				found = true;
				button->setFileItem(*item);
				break;
			}
		}

		if(!found && !mExcludedList.contains(item->url().url()) )
		{
			MediumButton *button = new MediumButton(this, *item);
			button->show();
			mButtonList.append(button);
		}
	}

	arrangeButtons();
}

void MediaApplet::slotDeleteItem(KFileItem *fileItem)
{
	kDebug()<<"MediumApplet::slotDeleteItem:"<< fileItem->url() << endl;

	MediumButtonList::iterator it;
	MediumButtonList::iterator end = mButtonList.end();
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;

		if(button->fileItem().url()==fileItem->url())
		{
			mButtonList.removeAll(button);
			delete button;
			break;
		}
	}

	arrangeButtons();
}

void MediaApplet::slotRefreshItems(const KFileItemList &entries)
{
	foreach(KFileItem *item, entries)
	{
		kDebug()<<"MediaApplet::slotRefreshItems:"<<(*item).url().url()<<endl;

		QString mimetype = (*item).mimetype();
		bool found = false;

		kDebug()<<"mimetype="<<mimetype<<endl;

		MediumButtonList::iterator it2;
		MediumButtonList::iterator end = mButtonList.end();
		for ( it2 = mButtonList.begin(); it2 != end; ++it2 )
		{
			MediumButton *button = *it2;

			if(button->fileItem().url()==(*item).url())
			{
				if(mExcludedTypesList.contains(mimetype))
				{
					mButtonList.removeAll(button);
					delete button;
				}
				else
				{
					button->setFileItem(*item);
				}
				found = true;
				break;
			}
		}

		if(!found && !mExcludedTypesList.contains(mimetype) && !mExcludedList.contains(item->url().url()) )
		{
			MediumButton *button = new MediumButton(this, *item);
			button->show();
			mButtonList.append(button);
		}
	}

	arrangeButtons();
}

void MediaApplet::positionChange(Plasma::Position)
{
	arrangeButtons();
}

void MediaApplet::loadConfig()
{
	KConfig *c = config();
	c->setGroup("General");

	if(c->hasKey("ExcludedTypes"))
	{
		mExcludedTypesList = c->readEntry("ExcludedTypes",QStringList(),';');
	}
	else
	{
		mExcludedTypesList.clear();
		mExcludedTypesList << "media/hdd_mounted";
		mExcludedTypesList << "media/hdd_unmounted";
		mExcludedTypesList << "media/nfs_mounted";
		mExcludedTypesList << "media/nfs_unmounted";
		mExcludedTypesList << "media/smb_mounted";
		mExcludedTypesList << "media/smb_unmounted";
	}

	if(c->hasKey("ExcludedMedia"))
	{
		mExcludedList = c->readEntry("ExcludedMedia",QStringList(),';');
	}
	else
	{
		mExcludedList.clear();
	}

}

void MediaApplet::saveConfig()
{
	KConfig *c = config();
	c->setGroup("General");

	c->writeEntry("ExcludedTypes", mExcludedTypesList, ';');
	c->writeEntry("ExcludedMedia", mExcludedList, ';');

	c->sync();
}

void MediaApplet::reloadList()
{
	mpDirLister->stop();

	if (!mButtonList.isEmpty())
	{
            mButtonList.clear();
	}

	mpDirLister->clearMimeFilter();
	mpDirLister->setMimeExcludeFilter(mExcludedTypesList);
	mpDirLister->openURL(KUrl("media:/"));
}

void MediaApplet::mousePressEvent(QMouseEvent *e)
{
        if(e->button()==Qt::RightButton)
	{
		KMenu menu(this);

		menu.addTitle(i18n("Media"));
		QAction *conf = menu.addAction(SmallIcon("configure"), i18n("&Configure..."));

		if ( menu.exec(this->mapToGlobal(e->pos())) == conf)
		{
			preferences();
		}
	}
}

#include "mediaapplet.moc"
