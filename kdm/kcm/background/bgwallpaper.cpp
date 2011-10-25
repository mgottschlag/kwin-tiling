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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include <config-workspace.h>

#include <QCheckBox>
#include <QPushButton>

#include <kfiledialog.h>
#include <kimageio.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "bgsettings.h"
#include "bgwallpaper.h"
#include "ui_bgwallpaper_ui.h"


/**** BGMultiWallpaperList ****/

class BGMultiWallpaperBase : public QWidget, public Ui::BGMultiWallpaperBase {
public:
    BGMultiWallpaperBase(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

BGMultiWallpaperList::BGMultiWallpaperList(QWidget *parent, const char *name)
    : QListWidget(parent)
{
    setObjectName(name);
    setAcceptDrops(true);
    setSelectionMode(QListWidget::ExtendedSelection);
}


void BGMultiWallpaperList::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->setAccepted(KUrl::List::canDecode(ev->mimeData()));
}


void BGMultiWallpaperList::dropEvent(QDropEvent *ev)
{
    QStringList files;
    const KUrl::List urls(KUrl::List::fromMimeData(ev->mimeData()));
    for (KUrl::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it) {
        // TODO: Download remote files
        if ((*it).isLocalFile())
            files.append((*it).toLocalFile());
    }
    addItems(files);
}

bool BGMultiWallpaperList::hasSelection()
{
    for (int i = 0; i < count(); i++) {
        if (item(i) && item(i)->isSelected())
            return true;
    }
    return false;
}

void BGMultiWallpaperList::ensureSelectionVisible()
{
    // KDE 4 - I believe the three lines below are redundant
    //
    //for (int i = topItem(); i < topItem() + numItemsVisible() - 1; i++)
    //    if (item(i) && item(i)->isSelected())
    //        return;

    for (int i = 0; i < count(); i++)
        if (item(i) && item(i)->isSelected()) {
            scrollToItem(item(i) , QAbstractItemView::PositionAtTop);
            return;
        }
}

/**** BGMultiWallpaperDialog ****/

BGMultiWallpaperDialog::BGMultiWallpaperDialog(KBackgroundSettings *settings,
        QWidget *parent, const char *name)
    : KDialog(parent), m_pSettings(settings)
{
    setObjectName(name);
    setModal(true);
    setCaption(i18n("Setup Slide Show"));
    setButtons(Ok | Cancel);

    dlg = new BGMultiWallpaperBase(this);
    setMainWidget(dlg);

    dlg->m_spinInterval->setRange(1, 99999);
    dlg->m_spinInterval->setSingleStep(15);
    dlg->m_spinInterval->setSuffix(ki18np(" minute", " minutes"));

    // Load
    dlg->m_spinInterval->setValue(qMax(1, m_pSettings->wallpaperChangeInterval()));

    dlg->m_listImages->addItems(m_pSettings->wallpaperList());

    if (m_pSettings->multiWallpaperMode() == KBackgroundSettings::Random)
        dlg->m_cbRandom->setChecked(true);

    connect(dlg->m_buttonAdd, SIGNAL(clicked()), SLOT(slotAdd()));
    connect(dlg->m_buttonRemove, SIGNAL(clicked()), SLOT(slotRemove()));
    connect(dlg->m_buttonMoveUp, SIGNAL(clicked()), SLOT(slotMoveUp()));
    connect(dlg->m_buttonMoveDown, SIGNAL(clicked()), SLOT(slotMoveDown()));
    connect(dlg->m_listImages, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(slotItemSelected(QListWidgetItem*)));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    dlg->m_buttonRemove->setEnabled(false);
    dlg->m_buttonMoveUp->setEnabled(false);
    dlg->m_buttonMoveDown->setEnabled(false);

}

void BGMultiWallpaperDialog::slotItemSelected(QListWidgetItem *)
{
    dlg->m_buttonRemove->setEnabled(dlg->m_listImages->hasSelection());
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::setEnabledMoveButtons()
{
    bool hasSelection = dlg->m_listImages->hasSelection();
    QListWidgetItem *item;

    item = dlg->m_listImages->item(0);
    dlg->m_buttonMoveUp->setEnabled(hasSelection && item && !item->isSelected());
    item  = dlg->m_listImages->item(dlg->m_listImages->count() - 1);
    dlg->m_buttonMoveDown->setEnabled(hasSelection && item && !item->isSelected());
}

void BGMultiWallpaperDialog::slotAdd()
{
    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    mimeTypes += "image/svg+xml";

    QStringList lstWallpaper = KGlobal::dirs()->findDirs("wallpaper", "");
    KFileDialog fileDialog((lstWallpaper.count() > 0) ? lstWallpaper.first() : QString(),
                           mimeTypes.join(" "), this);

    fileDialog.setCaption(i18n("Select Image"));
    KFile::Modes mode = KFile::Files |
                        KFile::Directory |
                        KFile::ExistingOnly |
                        KFile::LocalOnly;
    fileDialog.setMode(mode);
    fileDialog.exec();
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty())
        return;

    dlg->m_listImages->addItems(files);
}

void BGMultiWallpaperDialog::slotRemove()
{
    int current = -1;
    for (int i = 0; i < dlg->m_listImages->count();) {
        QListWidgetItem *item = dlg->m_listImages->item(i);
        if (item && item->isSelected()) {
            delete dlg->m_listImages->takeItem(i);
            if (current == -1)
                current = i;
        } else
            i++;
    }
    if ((current != -1) && (current < (signed)dlg->m_listImages->count()))
        dlg->m_listImages->item(current)->setSelected(true);

    dlg->m_buttonRemove->setEnabled(dlg->m_listImages->hasSelection());

    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotMoveUp()
{
    for (int i = 1; i < dlg->m_listImages->count(); i++) {
        QListWidgetItem *item = dlg->m_listImages->item(i);
        if (item && item->isSelected()) {
            dlg->m_listImages->takeItem(i);
            dlg->m_listImages->insertItem(i - 1 , item);
        }
    }
    dlg->m_listImages->ensureSelectionVisible();
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotMoveDown()
{
    for (int i = dlg->m_listImages->count() - 1; i > 0; i--) {
        QListWidgetItem *item = dlg->m_listImages->item(i - 1);
        if (item && item->isSelected()) {
            dlg->m_listImages->takeItem(i - 1);
            dlg->m_listImages->insertItem(i , item);
        }
    }
    dlg->m_listImages->ensureSelectionVisible();
    setEnabledMoveButtons();
}

void BGMultiWallpaperDialog::slotOk()
{
    QStringList lst;
    for (int i = 0; i < dlg->m_listImages->count(); i++)
        lst.append(dlg->m_listImages->item(i)->text());
    m_pSettings->setWallpaperList(lst);
    m_pSettings->setWallpaperChangeInterval(dlg->m_spinInterval->value());
    if (dlg->m_cbRandom->isChecked())
        m_pSettings->setMultiWallpaperMode(KBackgroundSettings::Random);
    else
        m_pSettings->setMultiWallpaperMode(KBackgroundSettings::InOrder);

    KDialog::accept();
}

#include "bgwallpaper.moc"
