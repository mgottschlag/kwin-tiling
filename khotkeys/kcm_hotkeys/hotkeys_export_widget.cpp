/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "hotkeys_export_widget.h"

KHotkeysExportWidget::KHotkeysExportWidget(QWidget *parent)
    : QWidget(parent)
    {
    ui.setupUi(this);
    }


KHotkeysExportWidget::~KHotkeysExportWidget()
    {
    }


KHotkeysExportDialog::KHotkeysExportDialog(QWidget *parent)
    : KDialog(parent)
    {
    setCaption(i18n("Export Group"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    w = new KHotkeysExportWidget(this);
    setMainWidget(w);
    }


KHotkeysExportDialog::~KHotkeysExportDialog()
    {}


bool
KHotkeysExportDialog::allowMerging() const
    {
    return w->ui.allowMerging->isChecked();
    }


QString
KHotkeysExportDialog::importId() const
    {
    return w->ui.id->text();
    }


void
KHotkeysExportDialog::setAllowMerging(bool allow)
    {
    w->ui.allowMerging->setChecked(allow);
    }


void
KHotkeysExportDialog::setImportId(const QString &id)
    {
    w->ui.id->setText(id);
    }


int
KHotkeysExportDialog::state() const
    {
    return w->ui.state->currentIndex();
    }


KUrl
KHotkeysExportDialog::url() const
    {
    return w->ui.filename->url();
    }


#include "moc_hotkeys_export_widget.cpp"
