/* Preview widget for KDE Display color scheme setup module
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

#include "previewwidget.h"

#include "ui_preview.h"

PreviewWidget::PreviewWidget(QWidget *parent) : QFrame(parent)
{
    Ui::preview ui;
    ui.setupUi(this);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    // set correct colors on... lots of things
    ui.frame->setBackgroundRole(QPalette::Base);
    ui.viewWidget->setBackgroundRole(QPalette::Base);
    ui.labelView0->setBackgroundRole(QPalette::Base);
    ui.labelView3->setBackgroundRole(QPalette::Base);
    ui.labelView4->setBackgroundRole(QPalette::Base);
    ui.labelView2->setBackgroundRole(QPalette::Base);
    ui.labelView1->setBackgroundRole(QPalette::Base);
    ui.labelView5->setBackgroundRole(QPalette::Base);
    ui.labelView6->setBackgroundRole(QPalette::Base);
    ui.labelView7->setBackgroundRole(QPalette::Base);
    ui.selectionWidget->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection0->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection3->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection4->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection2->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection1->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection5->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection6->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection7->setBackgroundRole(QPalette::Highlight);

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
}

PreviewWidget::~PreviewWidget()
{
}

#include "previewwidget.moc"
