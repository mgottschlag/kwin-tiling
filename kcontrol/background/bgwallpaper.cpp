/* vi: ts=8 sts=4 sw=4

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <qcheckbox.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <klocale.h>

#include "bgsettings.h"
#include "bgwallpaper.h"
#include "bgwallpaper_ui.h"

/**** BGMultiWallpaperList ****/

BGMultiWallpaperList::BGMultiWallpaperList(QWidget *parent, const char *name)
	: QListBox(parent, name)
{
   setAcceptDrops(true);
   setSelectionMode(QListBox::Multi);
}


void BGMultiWallpaperList::dragEnterEvent(QDragEnterEvent *ev)
{
   ev->accept(QUriDrag::canDecode(ev));
}


void BGMultiWallpaperList::dropEvent(QDropEvent *ev)
{
   QStringList files;
   QUriDrag::decodeLocalFiles(ev, files);
   insertStringList(files);
}

/**** BGMultiWallpaperDialog ****/

BGMultiWallpaperDialog::BGMultiWallpaperDialog(KBackgroundSettings *settings,
	QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, i18n("Setup Slide Show"),
	Ok | Cancel, Ok, true), m_pSettings(settings)
{
   dlg = new BGMultiWallpaperBase(this);
   setMainWidget(dlg);

   dlg->m_spinInterval->setRange(1, 240);
   dlg->m_spinInterval->setSteps(1, 15);
   dlg->m_spinInterval->setSuffix(i18n(" minutes"));

   // Load
   dlg->m_spinInterval->setValue(QMAX(1,m_pSettings->wallpaperChangeInterval()));

   dlg->m_listImages->insertStringList(m_pSettings->wallpaperList());

   if (m_pSettings->multiWallpaperMode() == KBackgroundSettings::Random)
      dlg->m_cbRandom->setChecked(true);

   connect(dlg->m_buttonAdd, SIGNAL(clicked()), SLOT(slotAdd()));
   connect(dlg->m_buttonRemove, SIGNAL(clicked()), SLOT(slotRemove()));
   connect(dlg->m_listImages,  SIGNAL(clicked ( QListBoxItem * )), SLOT(slotItemSelected( QListBoxItem *)));
   dlg->m_buttonRemove->setEnabled( false );

}

void BGMultiWallpaperDialog::slotItemSelected( QListBoxItem * item)
{
    dlg->m_buttonRemove->setEnabled( item );
}

void BGMultiWallpaperDialog::slotAdd()
{
    KFileDialog fileDialog(KGlobal::dirs()->findDirs("wallpaper", "").first(),
			   KImageIO::pattern(), this,
			   0L, true);

    fileDialog.setCaption(i18n("Select Image"));
    KFile::Mode mode = static_cast<KFile::Mode> (KFile::Files |
                                                 KFile::Directory |
                                                 KFile::ExistingOnly |
                                                 KFile::LocalOnly);
    fileDialog.setMode(mode);
    fileDialog.exec();
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty())
	return;

    dlg->m_listImages->insertStringList(files);
}

void BGMultiWallpaperDialog::slotRemove()
{
    for ( unsigned i = 0; i < dlg->m_listImages->count();)
    {
        QListBoxItem * item = dlg->m_listImages->item( i );
        if ( item && item->isSelected())
        {
            dlg->m_listImages->removeItem(i);
        }
        else
            i++;
    }
    dlg->m_buttonRemove->setEnabled( dlg->m_listImages->selectedItem () );
}

void BGMultiWallpaperDialog::slotOk()
{
    QStringList lst;
    for (unsigned i=0; i < dlg->m_listImages->count(); i++)
	lst.append(dlg->m_listImages->text(i));
    m_pSettings->setWallpaperList(lst);
    m_pSettings->setWallpaperChangeInterval(dlg->m_spinInterval->value());
    if (dlg->m_cbRandom->isChecked())
       m_pSettings->setMultiWallpaperMode(KBackgroundSettings::Random);
    else
       m_pSettings->setMultiWallpaperMode(KBackgroundSettings::InOrder);
    accept();
}


#include "bgwallpaper.moc"
