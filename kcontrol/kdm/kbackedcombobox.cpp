/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 2004-2005 Oswald Buddenhagen <ossi@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kbackedcombobox.h"

void KBackedComboBox::insertItem( const QString &id, const QString &name )
{
    id2name[id] = name;
    name2id[name] = id;
    KComboBox::addItem( name );
}

void KBackedComboBox::setCurrentId( const QString &id )
{
    if (id2name.contains( id ))
	setCurrentItem( id2name[id] );
    else
	setCurrentIndex( 0 );
}

const QString &KBackedComboBox::currentId() const
{
    return name2id[currentText()];
}
