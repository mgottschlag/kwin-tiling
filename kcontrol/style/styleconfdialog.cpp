/*
 * KCMStyle's container dialog for custom style setup dialogs
 *
 * (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "styleconfdialog.h"
#include <klocale.h>

StyleConfigDialog::StyleConfigDialog(QWidget* parent, QString styleName):
  KDialogBase(parent, "StyleConfigDialog",
           true, /*modal*/
           i18n("Configure %1").arg(styleName),
           KDialogBase::Default | KDialogBase::Ok | KDialogBase::Cancel,
           KDialogBase::Cancel)
{
  m_dirty = false;
  connect( this, SIGNAL( defaultClicked() ), this, SIGNAL( defaults() ));
  connect( this, SIGNAL( okClicked() ), this, SIGNAL( save() ));
}

bool StyleConfigDialog::isDirty() const
{
  return m_dirty;
}

void StyleConfigDialog::setDirty(bool dirty)
{
  m_dirty = dirty;
}

#include <styleconfdialog.moc>
