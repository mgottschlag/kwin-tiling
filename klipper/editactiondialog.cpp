/*
   This file is part of the KDE project
   Copyright (C) 2009 by Dmitry Suzdalev <dimsuz@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "editactiondialog.h"

#include <KDebug>

#include "urlgrabber.h"

#include "ui_editactiondialog.h"

EditActionDialog::EditActionDialog(QWidget* parent)
    : KDialog(parent)
{
    setCaption(i18n("Action Properties"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget* dlgWidget = new QWidget(this);
    m_ui = new Ui::EditActionDialog;
    m_ui->setupUi(dlgWidget);

    m_ui->pbAddCommand->setIcon(KIcon("list-add"));
    m_ui->pbRemoveCommand->setIcon(KIcon("list-remove"));

    setMainWidget(dlgWidget);
}

EditActionDialog::~EditActionDialog()
{
    delete m_ui;
}

void EditActionDialog::setAction(ClipAction* act)
{
    m_action = act;

    updateWidgets();
}

void EditActionDialog::updateWidgets()
{
    if (!m_action) {
        kDebug() << "no action to edit was set";
        return;
    }

    m_ui->leRegExp->setText(m_action->regExp());
    m_ui->leDescription->setText(m_action->description());
}

#include "editactiondialog.moc"
