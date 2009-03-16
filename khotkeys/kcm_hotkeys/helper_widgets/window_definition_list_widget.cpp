/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

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

#include "window_definition_list_widget.h"
#include "window_definition_widget.h"
#include "windows_helper/window_selection_rules.h"

#include "kdebug.h"

WindowDefinitionListWidget::WindowDefinitionListWidget(QWidget *parent)
    :   HotkeysWidgetIFace(parent)
        ,_windowdefs(NULL)
        ,_working(NULL)
        ,_changed(false)
    {
    ui.setupUi(this);

    connect(
            ui.edit_button, SIGNAL(clicked(bool)),
            SLOT(slotEdit(bool)));

    connect(
            ui.delete_button, SIGNAL(clicked(bool)),
            SLOT(slotDelete(bool)));

    connect(
            ui.duplicate_button, SIGNAL(clicked(bool)),
            SLOT(slotDuplicate(bool)));

    connect(
            ui.new_button, SIGNAL(clicked(bool)),
            SLOT(slotNew(bool)));
    }


WindowDefinitionListWidget::WindowDefinitionListWidget(KHotKeys::Windowdef_list *windowdef, QWidget *parent)
    :   HotkeysWidgetIFace(parent)
        ,_windowdefs(NULL)
        ,_working(NULL)
        ,_changed(false)
    {
    ui.setupUi(this);

    setWindowDefinitions(windowdef);

    connect(
            ui.edit_button, SIGNAL(clicked(bool)),
            SLOT(slotEdit(bool)));

    connect(
            ui.delete_button, SIGNAL(clicked(bool)),
            SLOT(slotDelete(bool)));

    connect(
            ui.duplicate_button, SIGNAL(clicked(bool)),
            SLOT(slotDuplicate(bool)));

    connect(
            ui.new_button, SIGNAL(clicked(bool)),
            SLOT(slotNew(bool)));
    }


WindowDefinitionListWidget::~WindowDefinitionListWidget()
    {
    delete _working;
    }


void WindowDefinitionListWidget::doCopyFromObject()
    {
    // We are asked to copy again from object. Recreate our working copy
    if (_working) delete _working;
    _working = _windowdefs->copy();

    ui.comment->setText(_working->comment());

    for ( KHotKeys::Windowdef_list::ConstIterator it(_working->constBegin());
            it != _working->constEnd();
            ++it)
        {
        new QListWidgetItem((*it)->description(), ui.list);
        }

    emitChanged(false);
    }


void WindowDefinitionListWidget::doCopyToObject()
    {
    // Delete the old content
    qDeleteAll(*_windowdefs);
    _windowdefs->clear();

    _windowdefs->set_comment(ui.comment->text());

    for (int i=0; i<_working->size(); ++i)
        {
        _windowdefs->append(_working->at(i)->copy());
        }

    // Reset our _changed state
    _changed = false;
    emitChanged(false);
    }


void WindowDefinitionListWidget::emitChanged( bool chgd )
    {
    if (_changed == chgd)
        return;

    // emitChanged will never reset _changed to false because we have
    // currently no way to compare the contents of _working and _windowdefs.
    // That's why we say once changed -> always changed.
    _changed = chgd || _changed;

    emit changed(_changed);
    }



bool WindowDefinitionListWidget::isChanged() const
    {
    return _changed;
    }


void WindowDefinitionListWidget::slotDelete(bool)
    {
    // TODO: Deactivate buttons if nothing is selected
    if (ui.list->currentRow() == -1)
        return;

    Q_ASSERT(ui.list->currentRow() < _working->count());

    KHotKeys::Windowdef *def = _working->at(ui.list->currentRow());
    KHotKeys::Windowdef_simple *sim = dynamic_cast<KHotKeys::Windowdef_simple*>(def);
    Q_ASSERT(sim);

    // Remove it from the list
    ui.list->takeItem(ui.list->currentRow());

    // delete it
    _working->removeAll(sim);
    delete sim;

    emitChanged(true);

    return;
    }


void WindowDefinitionListWidget::slotDuplicate(bool)
    {
    // TODO: Deactivate buttons if nothing is selected
    if (ui.list->currentRow() == -1)
        return;

    Q_ASSERT(ui.list->currentRow() < _working->count());

    // Get the template
    KHotKeys::Windowdef *def = _working->at(ui.list->currentRow());
    KHotKeys::Windowdef_simple *orig = dynamic_cast<KHotKeys::Windowdef_simple*>(def);
    Q_ASSERT(orig);

    // Create a copy
    KHotKeys::Windowdef_simple *sim = orig->copy();
    Q_ASSERT(sim);

    WindowDefinitionDialog dialog(sim, this);
    switch (dialog.exec())
        {
        case QDialog::Accepted:
            // Update our list if necessary
            new QListWidgetItem(sim->description(), ui.list);
            _working->append(sim);
            emitChanged(true);
            break;

        case QDialog::Rejected:
            // Nothing to do
            delete sim;
            break;

        default:
            Q_ASSERT(false);
            delete sim;
        }
    }


void WindowDefinitionListWidget::slotEdit(bool)
    {
    // TODO: Deactivate buttons if nothing is selected
    if (ui.list->currentRow() == -1)
        return;

    Q_ASSERT(ui.list->currentRow() < _working->count());

    QListWidgetItem *item = ui.list->currentItem();
    KHotKeys::Windowdef *def = _working->at(ui.list->currentRow());
    KHotKeys::Windowdef_simple *sim = dynamic_cast<KHotKeys::Windowdef_simple*>(def);

    Q_ASSERT(sim);
    if (!sim) return;

    WindowDefinitionDialog dialog(sim, this);
    switch (dialog.exec())
        {
        case QDialog::Accepted:
            // Update our list if necessary
            item->setText(sim->description());
            emitChanged(true);
            break;

        case QDialog::Rejected:
            // Nothing to do
            break;

        default:
            Q_ASSERT(false);
        }
    }


void WindowDefinitionListWidget::slotNew(bool)
    {
    KHotKeys::Windowdef_simple *sim = new KHotKeys::Windowdef_simple();

    WindowDefinitionDialog dialog(sim, this);
    switch (dialog.exec())
        {
        case QDialog::Accepted:
            // Update our list if necessary
            new QListWidgetItem(sim->description(), ui.list);
            _working->append(sim);
            emitChanged(true);
            break;

        case QDialog::Rejected:
            // Nothing to do
            delete sim;
            break;

        default:
            Q_ASSERT(false);
            delete sim;
        }
    }


void WindowDefinitionListWidget::setWindowDefinitions(KHotKeys::Windowdef_list *list)
    {
    Q_ASSERT(list);
    _windowdefs = list;
    }

#include "moc_window_definition_list_widget.cpp"
