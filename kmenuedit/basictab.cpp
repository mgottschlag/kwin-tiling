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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qgroupbox.h>
#include <qhbox.h>

#include <klocale.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kicondialog.h>
#include <kdesktopfile.h>

#include "basictab.h"
#include "basictab.moc"

BasicTab::BasicTab( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
    QGridLayout *layout = new QGridLayout(this, 4, 2,
					  KDialog::marginHint(),
					  KDialog::spacingHint());

    // general group
    QGroupBox *general_group = new QGroupBox(this);
    QGridLayout *grid = new QGridLayout(general_group, 4, 2,
					KDialog::marginHint(),
					KDialog::spacingHint());

    // setup labels
    grid->addWidget(new QLabel(i18n("Name"), general_group), 0, 0);
    grid->addWidget(new QLabel(i18n("Comment"), general_group), 1, 0);
    grid->addWidget(new QLabel(i18n("Command"), general_group), 2, 0);
    grid->addWidget(new QLabel(i18n("Type"), general_group), 3, 0);

    // setup line inputs
    _nameEdit = new KLineEdit(general_group);
    _commentEdit = new KLineEdit(general_group);
    _execEdit = new KLineEdit(general_group);
    _typeEdit = new KLineEdit(general_group);

    // connect line inputs
    connect(_nameEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    connect(_commentEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    connect(_execEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    connect(_typeEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));

    // add line inputs to the grid
    grid->addWidget(_nameEdit, 0, 1);
    grid->addWidget(_commentEdit, 1, 1);
    grid->addWidget(_execEdit, 2, 1);
    grid->addWidget(_typeEdit, 3, 1);

    // add the general group to the main layout
    layout->addMultiCellWidget(general_group, 0, 0, 0, 1);

    // path group
    _path_group = new QGroupBox(this);
    QVBoxLayout *vbox = new QVBoxLayout(_path_group, KDialog::marginHint(),
					KDialog::spacingHint());

    QHBox *hbox = new QHBox(_path_group);
    (void) new QLabel(i18n("Work Path"), hbox);
    hbox->setSpacing(KDialog::spacingHint());

    _pathEdit = new KLineEdit(hbox);
    connect(_pathEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    vbox->addWidget(hbox);
    layout->addWidget(_path_group, 1, 0);

    // setup icon button
    _iconButton = new KIconButton(this);
    _iconButton->setFixedSize(52,52);
    connect(_iconButton, SIGNAL(clicked()), SIGNAL(changed()));
    layout->addWidget(_iconButton, 1, 1);

    // terminal group
    _term_group = new QGroupBox(this);
    vbox = new QVBoxLayout(_term_group, KDialog::marginHint(),
			   KDialog::spacingHint());

    _terminalCB = new QCheckBox(i18n("Run in terminal"), _term_group);
    connect(_terminalCB, SIGNAL(clicked()), SLOT(termcb_clicked()));
    vbox->addWidget(_terminalCB);

    hbox = new QHBox(_term_group);
    (void) new QLabel(i18n("Terminal Options"), hbox);
    hbox->setSpacing(KDialog::spacingHint());
    _termOptEdit = new KLineEdit(hbox);
    connect(_termOptEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    vbox->addWidget(hbox);
    layout->addMultiCellWidget(_term_group, 2, 2, 0, 1);

    _termOptEdit->setEnabled(false);

    // uid group
    _uid_group = new QGroupBox(this);
    vbox = new QVBoxLayout(_uid_group, KDialog::marginHint(),
			   KDialog::spacingHint());

    _uidCB = new QCheckBox(i18n("Run as a different user"), _uid_group);
    connect(_uidCB, SIGNAL(clicked()), SLOT(uidcb_clicked()));
    vbox->addWidget(_uidCB);

    hbox = new QHBox(_uid_group);
    (void) new QLabel(i18n("Username"), hbox);
    hbox->setSpacing(KDialog::spacingHint());
    _uidEdit = new KLineEdit(hbox);
    connect(_uidEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(slotChanged(const QString&)));
    vbox->addWidget(hbox);
    layout->addMultiCellWidget(_uid_group, 3, 3, 0, 1);

    _uidEdit->setEnabled(false);

    layout->setRowStretch(0, 2);
}

void BasicTab::setDesktopFile(const QString& desktopFile)
{
    _desktopFile = desktopFile;

    KDesktopFile df(desktopFile);

    _nameEdit->setText(df.readName());
    _commentEdit->setText(df.readComment());
    _iconButton->setIcon(df.readIcon());

    // is desktopFile a .desktop file?
    bool isDF = desktopFile.find(".desktop") > 0;

    // set only basic attributes if it is not a .desktop file
    _execEdit->setEnabled(isDF);
    _typeEdit->setEnabled(isDF);
    _path_group->setEnabled(isDF);
    _term_group->setEnabled(isDF);
    _uid_group->setEnabled(isDF);

    if (!isDF) return;

    _execEdit->setText(df.readEntry("Exec"));
    _typeEdit->setText(df.readType());
    _pathEdit->setText(df.readPath());
    _termOptEdit->setText(df.readEntry("TerminalOptions"));
    _uidEdit->setText(df.readEntry("X-KDE-Username"));

    if(df.readNumEntry("Terminal", 0) == 1)
	_terminalCB->setChecked(true);
    else
	_terminalCB->setChecked(false);

    _uidCB->setChecked(df.readBoolEntry("X-KDE-SubstituteUID", false));

    _termOptEdit->setEnabled(_terminalCB->isChecked());
    _uidEdit->setEnabled(_uidCB->isChecked());
}

void BasicTab::apply()
{
    QString local = locateLocal("apps", _desktopFile);

    KDesktopFile df(local);
    df.writeEntry("Name", _nameEdit->text());
    df.writeEntry("Comment", _commentEdit->text());
    df.writeEntry("Icon", _iconButton->icon());

    if(_desktopFile.find(".desktop") < 0)
	{
	    df.sync();
	    return;
	}

    df.writeEntry("Exec", _execEdit->text());
    df.writeEntry("Type", _typeEdit->text());
    df.writeEntry("Path", _pathEdit->text());

    if (_terminalCB->isChecked())
	df.writeEntry("Terminal", 1);
    else
	df.writeEntry("Terminal", 0);

    df.writeEntry("TerminalOptions", _termOptEdit->text());
    df.writeEntry("X-KDE-SubstituteUID", _uidCB->isChecked());
    df.writeEntry("X-KDE-Username", _uidEdit->text());

    df.sync();
}

void BasicTab::reset()
{
    if(_desktopFile != "")
	setDesktopFile(_desktopFile);
}

void BasicTab::slotChanged(const QString&)
{
    emit changed();
}

void BasicTab::termcb_clicked()
{
    _termOptEdit->setEnabled(_terminalCB->isChecked());
    emit changed();
}

void BasicTab::uidcb_clicked()
{
    _uidEdit->setEnabled(_uidCB->isChecked());
    emit changed();
}
