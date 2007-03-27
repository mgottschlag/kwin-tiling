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

#include "preferencesdialog.h"

#include <klocale.h>
#include <kmimetype.h>
#include <kvbox.h>
#include <QLinkedList>
#include <k3listview.h>

class MediumTypeItem : public Q3CheckListItem
{
public:
	MediumTypeItem(Q3ListView *parent, const QString name,
	               const QString mimetype)
		: Q3CheckListItem(parent, name, CheckBox),
		  mMimeType(mimetype) { }

	const QString &mimeType() const { return mMimeType; }

private:
	QString mMimeType;
};

class MediumItem : public Q3CheckListItem
{
public:
	MediumItem(Q3ListView *parent, const QString name,
	           const KFileItem medium)
		: Q3CheckListItem(parent, name, CheckBox),
		  mMedium(medium) { }

	const QString itemURL() const { return mMedium.url().url(); }

private:
	KFileItem mMedium;
};



PreferencesDialog::PreferencesDialog(KFileItemList media, QWidget *parent,
                                     const char *name)
	: KPageDialog( parent ),
	  mMedia(media)
{
  setFaceType( Tabbed );
  setCaption( i18n("Media Applet Preferences") );
  setButtons( Ok|Cancel|Default );
  setObjectName( name );
  setModal( true );

	KVBox *types_page = new KVBox( this );
  addPage( types_page, i18n("Medium Types") );
	mpMediumTypesListView = new K3ListView(types_page);

	//mpMediumTypesListView->setFullWidth(true);
	mpMediumTypesListView->addColumn( i18n("Types to Display") );
	mpMediumTypesListView->setWhatsThis( i18n("Deselect the medium types which you do not want to see in the applet"));



	KVBox *media_page = new KVBox( this );
  addPage( media_page, i18n("Media") );
	mpMediaListView = new K3ListView(media_page);

	//mpMediaListView->setFullWidth(true);
	mpMediaListView->addColumn( i18n("Media to Display") );
	mpMediaListView->setWhatsThis( i18n("Deselect the media which you do not want to see in the applet"));

	slotDefault();
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::slotDefault()
{
	QStringList defaultExclude;

	defaultExclude << "media/hdd_mounted";
	defaultExclude << "media/hdd_unmounted";
	defaultExclude << "media/nfs_mounted";
	defaultExclude << "media/nfs_unmounted";
	defaultExclude << "media/smb_mounted";
	defaultExclude << "media/smb_unmounted";

	setExcludedMediumTypes(defaultExclude);
	setExcludedMedia(QStringList());
}

QStringList PreferencesDialog::excludedMediumTypes()
{
	QStringList excludedTypes;

	for(MediumTypeItem *it=static_cast<MediumTypeItem *>(mpMediumTypesListView->firstChild());
	    it; it=static_cast<MediumTypeItem *>(it->nextSibling()))
	{
		if(!it->isOn()) excludedTypes << it->mimeType();
	}

	return excludedTypes;
}

void PreferencesDialog::setExcludedMediumTypes(const QStringList& excludedTypesList)
{
	mpMediumTypesListView->clear();
	mpMediumTypesListView->setRootIsDecorated(false);

        foreach(const KMimeType::Ptr &mimeType, KMimeType::allMimeTypes())
        {
            if (mimeType->name().startsWith("media/"))
            {
                bool ok = excludedTypesList.contains(mimeType->name());
                MediumTypeItem *item = new MediumTypeItem(mpMediumTypesListView, mimeType->comment(), mimeType->name());
                item->setOn(ok);
            }
	}
}

QStringList PreferencesDialog::excludedMedia()
{
	QStringList excluded;

	for(MediumItem *it=static_cast<MediumItem *>(mpMediaListView->firstChild());
	    it; it=static_cast<MediumItem *>(it->nextSibling()))
	{
		if(!it->isOn()) excluded << it->itemURL();
	}

	return excluded;
}

void PreferencesDialog::setExcludedMedia(const QStringList& excludedList)
{
	mpMediaListView->clear();
	mpMediaListView->setRootIsDecorated(false);

	foreach( KFileItem *file, mMedia )
	{
		bool ok = !excludedList.contains(file->url().url());
		MediumItem *item = new MediumItem(mpMediaListView,
		                                  file->text(), *file);
		item->setOn(ok);
	}
}


#include "preferencesdialog.moc"
