/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "window_definition_widget.h"
#include "ui_window_definition_widget.h"

#include "windows_handler.h"
#include "helper_widgets/window_selector.h"
#include "windows_helper/window_selection_rules.h"

#include <KDebug>


WindowDefinitionWidget::WindowDefinitionWidget(KHotKeys::Windowdef_simple *windowdef, QWidget *parent)
    :   HotkeysWidgetIFace(parent)
        ,ui(new Ui::WindowDefinitionWidget)
        ,_windowdef(windowdef)
    {
    ui->setupUi(this);
    connect(ui->window_class_combo, SIGNAL(currentIndexChanged(int)),
            SLOT(slotWindowClassChanged(int)));
    connect(ui->window_title_combo, SIGNAL(currentIndexChanged(int)),
            SLOT(slotWindowTitleChanged(int)));
    connect(ui->window_role_combo, SIGNAL(currentIndexChanged(int)),
            SLOT(slotWindowRoleChanged(int)));
    connect(ui->autodetect, SIGNAL(clicked()),
            SLOT(slotAutoDetect()));

    // user changes -> isChanged for all others
    connect(
            ui->comment, SIGNAL(textChanged(QString)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->comment, "text" );

    connect(
            ui->window_class, SIGNAL(textChanged(QString)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->window_class, "window_class" );

    connect(
            ui->window_role, SIGNAL(textChanged(QString)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->window_role, "window_role" );

    connect(
            ui->window_title, SIGNAL(textChanged(QString)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->window_title, "window_title" );

    connect(
            ui->type_dialog, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->type_dialog, "window_type_dialog" );

    connect(
            ui->type_dock, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->type_dock, "window_type_dock" );

    connect(
            ui->type_desktop, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->type_desktop, "window_type_desktop" );

    connect(
            ui->type_normal, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui->type_normal, "window_type_normal" );
    }


WindowDefinitionWidget::~WindowDefinitionWidget()
    { delete ui; }


void WindowDefinitionWidget::doCopyFromObject()
    {
    ui->comment->setText(_windowdef->comment());
    ui->window_class->setText(_windowdef->wclass());
    ui->window_class_combo->setCurrentIndex(_windowdef->wclass_match_type());
    ui->window_role->setText(_windowdef->role());
    ui->window_role_combo->setCurrentIndex(_windowdef->role_match_type());
    ui->window_title->setText(_windowdef->title());
    ui->window_title_combo->setCurrentIndex(_windowdef->title_match_type());
    ui->type_desktop->setChecked(
            _windowdef->type_match(KHotKeys::Windowdef_simple::WINDOW_TYPE_DESKTOP));
    ui->type_dialog->setChecked(
            _windowdef->type_match(KHotKeys::Windowdef_simple::WINDOW_TYPE_DIALOG));
    ui->type_dock->setChecked(
            _windowdef->type_match(KHotKeys::Windowdef_simple::WINDOW_TYPE_DOCK));
    ui->type_normal->setChecked(
            _windowdef->type_match(KHotKeys::Windowdef_simple::WINDOW_TYPE_NORMAL));
    }


void WindowDefinitionWidget::doCopyToObject()
    {
    _windowdef->set_comment(ui->comment->text());
    _windowdef->set_wclass(ui->window_class->text());
    _windowdef->set_wclass_match_type(static_cast<KHotKeys::Windowdef_simple::substr_type_t>(ui->window_class_combo->currentIndex()));
    _windowdef->set_role(ui->window_role->text());
    _windowdef->set_role_match_type(static_cast<KHotKeys::Windowdef_simple::substr_type_t>(ui->window_role_combo->currentIndex()));
    _windowdef->set_title(ui->window_title->text());
    _windowdef->set_title_match_type(static_cast<KHotKeys::Windowdef_simple::substr_type_t>(ui->window_title_combo->currentIndex()));
    int types = 0;
    if (ui->type_desktop->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DESKTOP;
    if (ui->type_dialog->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DIALOG;
    if (ui->type_dock->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DOCK;
    if (ui->type_normal->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_NORMAL;
    _windowdef->set_window_types(types);
    }


bool WindowDefinitionWidget::isChanged() const
    {
    if (   _windowdef->comment() != ui->comment->text()
        || _windowdef->wclass() != ui->window_class->text()
        || _windowdef->wclass_match_type() != ui->window_class_combo->currentIndex()
        || _windowdef->role() != ui->window_role->text()
        || _windowdef->role_match_type() != ui->window_role_combo->currentIndex()
        || _windowdef->title() != ui->window_title->text()
        || _windowdef->title_match_type() != ui->window_title_combo->currentIndex())
        {
        return true;
        }

    int types = 0;
    if (ui->type_desktop->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DESKTOP;
    if (ui->type_dialog->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DIALOG;
    if (ui->type_dock->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_DOCK;
    if (ui->type_normal->isChecked())
            types |= KHotKeys::Windowdef_simple::WINDOW_TYPE_NORMAL;
    kDebug() << _windowdef->window_types() << types;
    return _windowdef->window_types() != types;
    }


void WindowDefinitionWidget::slotAutoDetect()
    {
    KHotKeys::WindowSelector* sel = new KHotKeys::WindowSelector( this, SLOT(slotWindowSelected(WId)));
    sel->select();
    }


void WindowDefinitionWidget::slotWindowClassChanged(int index)
    {
    ui->window_class->setEnabled(index!=0);
    slotChanged("window_class");
    }


void WindowDefinitionWidget::slotWindowRoleChanged(int index)
    {
    ui->window_role->setEnabled(index!=0);
    slotChanged("window_role");
    }


void WindowDefinitionWidget::slotWindowSelected(WId window)
    {
    if (window)
        {
        KHotKeys::Window_data data( window );
        ui->window_title->setText( data.title );
        ui->window_role->setText( data.role );
        ui->window_class->setText( data.wclass );
        ui->type_normal->setChecked( data.type == NET::Normal );
        ui->type_dialog->setChecked( data.type == NET::Dialog );
        ui->type_dock->setChecked( data.type == NET::Dock );
        ui->type_desktop->setChecked( data.type == NET::Desktop );
        }
    }


void WindowDefinitionWidget::slotWindowTitleChanged(int index)
    {
    ui->window_title->setEnabled(index!=0);
    slotChanged("window_title");
    }


#include "moc_window_definition_widget.cpp"
