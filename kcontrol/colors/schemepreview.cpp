/* Scheme preview for KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "schemepreview.h"

#include "ui_schemepreview.h"

SchemePreview::SchemePreview(QWidget *parent) : QFrame(parent)
{
    Ui::schemePreview ui;
    ui.setupUi(this);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
}

SchemePreview::~SchemePreview()
{
}

#include "schemepreview.moc"
