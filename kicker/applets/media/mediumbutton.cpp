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

#include "mediumbutton.h"

#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <qdrawutil.h>
#include <QMenu>
#include <QStyle>
#include <QToolTip>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <kmessagebox.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krun.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kstandardaction.h>

#include <konq_operations.h>
#include <konq_popupmenu.h>
#include <konqmimedata.h>

MediumButton::MediumButton(QWidget *parent, const KFileItem &fileItem)
	: PanelPopupButton(parent), mActions(this), mFileItem(fileItem)
{
	mActions.setAssociatedWidget(this);

	QAction *a = KStandardAction::paste(this, SLOT(slotPaste()), this);
        mActions.addAction("pasteto", a);
	a->setShortcut(0);
	a = KStandardAction::copy(this, SLOT(slotCopy()), this);
        mActions.addAction("copy", a);
	a->setShortcut(0);

	resize(20, 20);

	setAcceptDrops(mFileItem.isWritable());

	setTitle(mFileItem.text());

	refreshType();

	mOpenTimer.setSingleShot(true);
	connect(&mOpenTimer, SIGNAL(timeout()), SLOT(slotDragOpen()));

	// Activate this code only if we find a way to have both an
	// action and a popup menu for the same kicker button
	//connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

	setPopup(new QMenu());
}

MediumButton::~MediumButton()
{
	QMenu *menu = popup();
	delete menu;
}

const KFileItem &MediumButton::fileItem() const
{
	return mFileItem;
}

void MediumButton::setFileItem(const KFileItem &fileItem)
{
	mFileItem.assign(fileItem);
	setAcceptDrops(mFileItem.isWritable());
	setTitle(mFileItem.text());
	refreshType();
}

void MediumButton::initPopup()
{
	QMenu *old_popup = popup();

	KFileItemList items;
	items.append(&mFileItem);

	KonqPopupMenu::KonqPopupFlags kpf =
		  KonqPopupMenu::ShowProperties
		| KonqPopupMenu::ShowNewWindow;

	KParts::BrowserExtension::PopupFlags bef =
		  KParts::BrowserExtension::DefaultPopupItems;

	KonqPopupMenu *new_popup = new KonqPopupMenu(0L, items,
	                                   KUrl("media:/"), mActions, 0L,
	                                   this, kpf, bef);
        // KPopupTitle removed in KDE4
//	KPopupTitle *title = new KPopupTitle(new_popup);
//	title->setTitle(mFileItem.text());

//	new_popup->insertItem(title, -1, 0);
        new_popup->insertItem(mFileItem.text(), -1, 0);

	setPopup(new_popup);

	if (old_popup!=0L) delete old_popup;
}

void MediumButton::refreshType()
{
    KMimeType::Ptr mime = mFileItem.determineMimeType();
    this->setToolTip( mime->comment());
    setIcon(mime->iconName(KUrl()));
}

// Activate this code only if we find a way to have both an
// action and a popup menu for the same kicker button
/*
void MediumButton::slotClicked()
{
	mFileItem.run();
}
*/

void MediumButton::slotPaste()
{
	KonqOperations::doPaste(this, mFileItem.url());
}

void MediumButton::slotCopy()
{
  bool dummy;
  QMimeData* mimeData = new QMimeData;
  KonqMimeData::populateMimeData( mimeData, mFileItem.url(), mFileItem.mostLocalUrl(dummy) );
  QApplication::clipboard()->setMimeData( mimeData );
}

void MediumButton::dragEnterEvent(QDragEnterEvent* e)
{
	if (mFileItem.isWritable())
	{
		mOpenTimer.start(1000);
		e->setAccepted(true);
	}
}

void MediumButton::dragLeaveEvent(QDragLeaveEvent* e)
{
	mOpenTimer.stop();

	PanelPopupButton::dragLeaveEvent( e );
}

void MediumButton::dropEvent(QDropEvent *e)
{
	mOpenTimer.stop();

	KonqOperations::doDrop(&mFileItem, mFileItem.url(), e, this);
}

void MediumButton::slotDragOpen()
{
	mFileItem.run();
}

QString MediumButton::tileName()
{
	return mFileItem.text();
}

void MediumButton::setPanelPosition(Plasma::Position position)
{
	switch(position)
	{
	case Plasma::Bottom:
		setPopupDirection(Plasma::Up);
		break;
	case Plasma::Top:
		setPopupDirection(Plasma::Down);
		break;
	case Plasma::Right:
		setPopupDirection(Plasma::Left);
		break;
	case Plasma::Left:
		setPopupDirection(Plasma::Right);
		break;
	case Plasma::Floating:
		setPopupDirection(Plasma::Floating);
		break;
	}
}

#include "mediumbutton.moc"
