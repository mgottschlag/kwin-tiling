/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qgroupbox.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kdialog.h>
#include <kkeybutton.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kicondialog.h>
#include <kdesktopfile.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <kkeydialog.h>
#include <kprocess.h>
#include <khbox.h>

#include "khotkeys.h"

#include "menuinfo.h"

#include "basictab.h"
#include "basictab.moc"

BasicTab::BasicTab( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
    _menuFolderInfo = 0;
    _menuEntryInfo = 0;

    QGridLayout *layout = new QGridLayout(this, 6, 2,
                                          KDialog::marginHint(),
                                          KDialog::spacingHint());

    // general group
    QGroupBox *general_group = new QGroupBox(this);
    QGridLayout *grid = new QGridLayout(general_group, 5, 2,
                                        KDialog::marginHint(),
                                        KDialog::spacingHint());

    general_group->setAcceptDrops(false);

    // setup line inputs
    _nameEdit = new KLineEdit(general_group);
    _nameEdit->setAcceptDrops(false);
    _descriptionEdit = new KLineEdit(general_group);
    _descriptionEdit->setAcceptDrops(false);
    _commentEdit = new KLineEdit(general_group);
    _commentEdit->setAcceptDrops(false);
    _execEdit = new KUrlRequester(general_group);
    _execEdit->lineEdit()->setAcceptDrops(false);
    _execEdit->setWhatsThis(i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the folder of the file to open\n"
    "%D - a list of folders\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the caption"));

    _launchCB = new QCheckBox(i18n("Enable &launch feedback"), general_group);
    _systrayCB = new QCheckBox(i18n("&Place in system tray"), general_group);

    // setup labels
    _nameLabel = new QLabel(_nameEdit, i18n("&Name:"), general_group);
    _descriptionLabel = new QLabel(_descriptionEdit, i18n("&Description:"), general_group);
    _commentLabel = new QLabel(_commentEdit, i18n("&Comment:"), general_group);
    _execLabel = new QLabel(_execEdit, i18n("Co&mmand:"), general_group);
    grid->addWidget(_nameLabel, 0, 0);
    grid->addWidget(_descriptionLabel, 1, 0);
    grid->addWidget(_commentLabel, 2, 0);
    grid->addWidget(_execLabel, 3, 0);

    // connect line inputs
    connect(_nameEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotChanged()));
    connect(_descriptionEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged()));
    connect(_commentEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotChanged()));
    connect(_execEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotChanged()));
    connect(_execEdit, SIGNAL(urlSelected(const QString&)),
            SLOT(slotExecSelected()));
    connect(_launchCB, SIGNAL(clicked()), SLOT(launchcb_clicked()));
    connect(_systrayCB, SIGNAL(clicked()), SLOT(systraycb_clicked()));

    // add line inputs to the grid
    grid->addMultiCellWidget(_nameEdit, 0, 0, 1, 1);
    grid->addMultiCellWidget(_descriptionEdit, 1, 1, 1, 1);
    grid->addMultiCellWidget(_commentEdit, 2, 2, 1, 2);
    grid->addMultiCellWidget(_execEdit, 3, 3, 1, 2);
    grid->addMultiCellWidget(_launchCB, 4, 4, 0, 2);
    grid->addMultiCellWidget(_systrayCB, 5, 5, 0, 2);

    // setup icon button
    _iconButton = new KIconButton(general_group);
    _iconButton->setFixedSize(56,56);
    _iconButton->setIconSize(48);
    connect(_iconButton, SIGNAL(iconChanged(QString)), SLOT(slotChanged()));
    grid->addMultiCellWidget(_iconButton, 0, 1, 2, 2);

    // add the general group to the main layout
    layout->addMultiCellWidget(general_group, 0, 0, 0, 1);

    // path group
    _path_group = new QGroupBox(this);
    QVBoxLayout *vbox = new QVBoxLayout(_path_group);
    vbox->setMargin(KDialog::marginHint());
    vbox->setSpacing(KDialog::spacingHint());

    QWidget *hbox = new QWidget(_path_group);
    QHBoxLayout *hboxLayout1 = new QHBoxLayout(hbox);
    hbox->setLayout(hboxLayout1);
    hboxLayout1->setSpacing(KDialog::spacingHint());

    _pathLabel = new QLabel(i18n("&Work path:"), hbox);
	hboxLayout1->addWidget(_pathLabel);
    _pathEdit = new KUrlRequester(hbox);
	hboxLayout1->addWidget(_pathEdit);
    _pathEdit->setMode(KFile::Directory | KFile::LocalOnly);
    _pathEdit->lineEdit()->setAcceptDrops(false);

    _pathLabel->setBuddy(_pathEdit);

    connect(_pathEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotChanged()));
    vbox->addWidget(hbox);
    layout->addMultiCellWidget(_path_group, 1, 1, 0, 1);

    // terminal group
    _term_group = new QGroupBox(this);
    vbox = new QVBoxLayout(_term_group);
    vbox->setMargin(KDialog::marginHint());
    vbox->setSpacing(KDialog::spacingHint());

    _terminalCB = new QCheckBox(i18n("Run in term&inal"), _term_group);
    connect(_terminalCB, SIGNAL(clicked()), SLOT(termcb_clicked()));
    vbox->addWidget(_terminalCB);

    hbox = new QWidget(_term_group);
    QHBoxLayout *hboxLayout2 = new QHBoxLayout(hbox);
    hbox->setLayout(hboxLayout2);
    hboxLayout2->setSpacing(KDialog::spacingHint());
    _termOptLabel = new QLabel(i18n("Terminal &options:"), hbox);
	hboxLayout2->addWidget(_termOptLabel);
    _termOptEdit = new KLineEdit(hbox);
	hboxLayout2->addWidget(_termOptEdit);
    _termOptEdit->setAcceptDrops(false);
    _termOptLabel->setBuddy(_termOptEdit);

    connect(_termOptEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotChanged()));
    vbox->addWidget(hbox);
    layout->addMultiCellWidget(_term_group, 2, 2, 0, 1);

    _termOptEdit->setEnabled(false);

    // uid group
    _uid_group = new QGroupBox(this);
    vbox = new QVBoxLayout(_uid_group);
    vbox->setMargin(KDialog::marginHint());
    vbox->setSpacing(KDialog::spacingHint());

    _uidCB = new QCheckBox(i18n("&Run as a different user"), _uid_group);
    connect(_uidCB, SIGNAL(clicked()), SLOT(uidcb_clicked()));
    vbox->addWidget(_uidCB);

    hbox = new QWidget(_uid_group);
    QHBoxLayout *hboxLayout3 = new QHBoxLayout(hbox);
    hbox->setLayout(hboxLayout3);
    hboxLayout3->setSpacing(KDialog::spacingHint());
    _uidLabel = new QLabel(i18n("&Username:"), hbox);
	hboxLayout3->addWidget(_uidLabel);
    _uidEdit = new KLineEdit(hbox);
	hboxLayout3->addWidget(_uidEdit);
    _uidEdit->setAcceptDrops(false);
    _uidLabel->setBuddy(_uidEdit);

    connect(_uidEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged()));
    vbox->addWidget(hbox);
    layout->addMultiCellWidget(_uid_group, 3, 3, 0, 1);

    _uidEdit->setEnabled(false);

    layout->setRowStretch(0, 2);

    // key binding group
    general_group_keybind = new QGroupBox(this);
    layout->addMultiCellWidget( general_group_keybind, 4, 4, 0, 1 );
    // dummy widget in order to make it look a bit better
    layout->addWidget( new QWidget(this), 5, 0 );
    layout->setRowStretch( 5, 4 );
    QGridLayout *grid_keybind = new QGridLayout(general_group_keybind, 3, 1,
                                                KDialog::marginHint(),
                                                KDialog::spacingHint());

    //_keyEdit = new KLineEdit(general_group_keybind);
    //_keyEdit->setReadOnly( true );
    //_keyEdit->setText( "" );
    //QPushButton* _keyButton = new QPushButton( i18n( "Change" ),
    //                                           general_group_keybind );
    //connect( _keyButton, SIGNAL( clicked()), this, SLOT( keyButtonPressed()));
    _keyEdit = new KKeyButton(general_group_keybind);
    grid_keybind->addWidget(new QLabel(_keyEdit, i18n("Current shortcut &key:"), general_group_keybind), 0, 0);
    connect( _keyEdit, SIGNAL(capturedShortcut(const KShortcut&)),
             this, SLOT(slotCapturedShortcut(const KShortcut&)));
    grid_keybind->addWidget(_keyEdit, 0, 1);
    //grid_keybind->addWidget(_keyButton, 0, 2 );

    if (!KHotKeys::present())
       general_group_keybind->hide();

    slotDisableAction();
}

void BasicTab::slotDisableAction()
{
    //disable all group at the begining.
    //because there is not file selected.
    _nameEdit->setEnabled(false);
    _descriptionEdit->setEnabled(false);
    _commentEdit->setEnabled(false);
    _execEdit->setEnabled(false);
    _launchCB->setEnabled(false);
    _systrayCB->setEnabled(false);
    _nameLabel->setEnabled(false);
    _descriptionLabel->setEnabled(false);
    _commentLabel->setEnabled(false);
    _execLabel->setEnabled(false);
    _path_group->setEnabled(false);
    _term_group->setEnabled(false);
    _uid_group->setEnabled(false);
    _iconButton->setEnabled(false);
    // key binding part
    general_group_keybind->setEnabled( false );
}

void BasicTab::enableWidgets(bool isDF, bool isDeleted)
{
    // set only basic attributes if it is not a .desktop file
    _nameEdit->setEnabled(!isDeleted);
    _descriptionEdit->setEnabled(!isDeleted);
    _commentEdit->setEnabled(!isDeleted);
    _iconButton->setEnabled(!isDeleted);
    _execEdit->setEnabled(isDF && !isDeleted);
    _launchCB->setEnabled(isDF && !isDeleted);
    _systrayCB->setEnabled(isDF && !isDeleted);
    _nameLabel->setEnabled(!isDeleted);
    _descriptionLabel->setEnabled(!isDeleted);
    _commentLabel->setEnabled(!isDeleted);
    _execLabel->setEnabled(isDF && !isDeleted);

    _path_group->setEnabled(isDF && !isDeleted);
    _term_group->setEnabled(isDF && !isDeleted);
    _uid_group->setEnabled(isDF && !isDeleted);
    general_group_keybind->setEnabled( isDF && !isDeleted );

    _termOptEdit->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());
    _termOptLabel->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());

    _uidEdit->setEnabled(isDF && !isDeleted && _uidCB->isChecked());
    _uidLabel->setEnabled(isDF && !isDeleted && _uidCB->isChecked());
}

void BasicTab::setFolderInfo(MenuFolderInfo *folderInfo)
{
    blockSignals(true);
    _menuFolderInfo = folderInfo;
    _menuEntryInfo = 0;

    _nameEdit->setText(folderInfo->caption);
    _descriptionEdit->setText(folderInfo->genericname);
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(folderInfo->comment);
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(folderInfo->icon);

    // clean all disabled fields and return
    _execEdit->lineEdit()->setText("");
    _pathEdit->lineEdit()->setText("");
    _termOptEdit->setText("");
    _uidEdit->setText("");
    _launchCB->setChecked(false);
    _systrayCB->setChecked(false);
    _terminalCB->setChecked(false);
    _uidCB->setChecked(false);
    _keyEdit->setShortcut(KShortcut());

    enableWidgets(false, folderInfo->hidden);
    blockSignals(false);
}

void BasicTab::setEntryInfo(MenuEntryInfo *entryInfo)
{
    blockSignals(true);
    _menuFolderInfo = 0;
    _menuEntryInfo = entryInfo;
    
    if (!entryInfo)
    {
       _nameEdit->setText(QString());
       _descriptionEdit->setText(QString());
       _commentEdit->setText(QString());
       _iconButton->setIcon(QString());

       // key binding part
       _keyEdit->setShortcut( KShortcut() );
       _execEdit->lineEdit()->setText(QString());
       _systrayCB->setChecked(false);

       _pathEdit->lineEdit()->setText(QString());
       _termOptEdit->setText(QString());
       _uidEdit->setText(QString());

       _launchCB->setChecked(false);
       _terminalCB->setChecked(false);
       _uidCB->setChecked(false);
       enableWidgets(true, true);
       blockSignals(false);
       return;
    }

    KDesktopFile *df = entryInfo->desktopFile();

    _nameEdit->setText(df->readName());
    _descriptionEdit->setText(df->readGenericName());
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(df->readComment());
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(df->readIcon());

    // key binding part
    if( KHotKeys::present())
    {
        _keyEdit->setShortcut( entryInfo->shortcut() );
    }

    QString temp = df->readPathEntry("Exec");
    if (temp.left(12) == "ksystraycmd ")
    {
      _execEdit->lineEdit()->setText(temp.right(temp.length()-12));
      _systrayCB->setChecked(true);
    }
    else
    {
      _execEdit->lineEdit()->setText(temp);
      _systrayCB->setChecked(false);
    }

    _pathEdit->lineEdit()->setText(df->readPath());
    _termOptEdit->setText(df->readEntry("TerminalOptions"));
    _uidEdit->setText(df->readEntry("X-KDE-Username"));

    if( df->hasKey( "StartupNotify" ))
        _launchCB->setChecked(df->readEntry("StartupNotify", QVariant(true)).toBool());
    else // backwards comp.
        _launchCB->setChecked(df->readEntry("X-KDE-StartupNotify", QVariant(true)).toBool());

    if(df->readEntry("Terminal", 0) == 1)
        _terminalCB->setChecked(true);
    else
        _terminalCB->setChecked(false);

    _uidCB->setChecked(df->readEntry("X-KDE-SubstituteUID", QVariant(false)).toBool());

    enableWidgets(true, entryInfo->hidden);
    blockSignals(false);
}

void BasicTab::apply()
{
    if (_menuEntryInfo)
    {
        _menuEntryInfo->setDirty();
        _menuEntryInfo->setCaption(_nameEdit->text());
        _menuEntryInfo->setDescription(_descriptionEdit->text());
        _menuEntryInfo->setIcon(_iconButton->icon());

        KDesktopFile *df = _menuEntryInfo->desktopFile();
        df->writeEntry("Comment", _commentEdit->text());
        if (_systrayCB->isChecked())
          df->writePathEntry("Exec", _execEdit->lineEdit()->text().prepend("ksystraycmd "));
        else
          df->writePathEntry("Exec", _execEdit->lineEdit()->text());

        df->writePathEntry("Path", _pathEdit->lineEdit()->text());

        if (_terminalCB->isChecked())
            df->writeEntry("Terminal", 1);
        else
            df->writeEntry("Terminal", 0);

        df->writeEntry("TerminalOptions", _termOptEdit->text());
        df->writeEntry("X-KDE-SubstituteUID", _uidCB->isChecked());
        df->writeEntry("X-KDE-Username", _uidEdit->text());
        df->writeEntry("StartupNotify", _launchCB->isChecked());
    }
    else
    {
        _menuFolderInfo->setCaption(_nameEdit->text());
        _menuFolderInfo->setGenericName(_descriptionEdit->text());
        _menuFolderInfo->setComment(_commentEdit->text());
        _menuFolderInfo->setIcon(_iconButton->icon());
    }
}

void BasicTab::slotChanged()
{
    if (signalsBlocked())
       return;
    apply();
    if (_menuEntryInfo)
       emit changed( _menuEntryInfo );
    else
       emit changed( _menuFolderInfo );
}

void BasicTab::launchcb_clicked()
{
    slotChanged();
}

void BasicTab::systraycb_clicked()
{
    slotChanged();
}

void BasicTab::termcb_clicked()
{
    _termOptEdit->setEnabled(_terminalCB->isChecked());
    _termOptLabel->setEnabled(_terminalCB->isChecked());
    slotChanged();
}

void BasicTab::uidcb_clicked()
{
    _uidEdit->setEnabled(_uidCB->isChecked());
    _uidLabel->setEnabled(_uidCB->isChecked());
    slotChanged();
}

void BasicTab::slotExecSelected()
{
    QString path = _execEdit->lineEdit()->text();
    if (!path.startsWith("'"))
        _execEdit->lineEdit()->setText(KProcess::quote(path));
}

void BasicTab::slotCapturedShortcut(const KShortcut& cut)
{
    if (signalsBlocked())
       return;

    if( KKeyChooser::checkGlobalShortcutsConflict( cut, true, topLevelWidget())
        || KKeyChooser::checkStandardShortcutsConflict( cut, true, topLevelWidget()))
        return;

    if ( KHotKeys::present() )
    {
       if (!_menuEntryInfo->isShortcutAvailable( cut ) )
       {
          KService::Ptr service;
          emit findServiceShortcut(cut, service);
          if (!service)
             service = KHotKeys::findMenuEntry(cut.toString());
          if (service)
          {
             KMessageBox::sorry(this, i18n("<qt>The key <b>%1</b> can not be used here because it is already used to activate <b>%2</b>.", cut.toString(), service->name()));
             return;
          }
          else
          {
             KMessageBox::sorry(this, i18n("<qt>The key <b>%1</b> can not be used here because it is already in use.", cut.toString()));
             return;
          }
       }
       _menuEntryInfo->setShortcut( cut );
    }
    _keyEdit->setShortcut(cut);
    if (_menuEntryInfo)
       emit changed( _menuEntryInfo );
}

