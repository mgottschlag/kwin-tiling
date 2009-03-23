/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "hotkeys_proxy_model.h"
#include "hotkeys_model.h"


struct KHotkeysProxyModelPrivate
    {
    KHotkeysProxyModelPrivate( KHotkeysProxyModel *host );

    //! Our host
    KHotkeysProxyModel *q;

    }; // class KHotkeysProxyModelPrivate


KHotkeysProxyModelPrivate::KHotkeysProxyModelPrivate( KHotkeysProxyModel *host )
    : q(host)
    {}



KHotkeysProxyModel::KHotkeysProxyModel( QObject *parent )
    : QSortFilterProxyModel(parent)
     ,d( new KHotkeysProxyModelPrivate(this) )
    {}


KHotkeysProxyModel::~KHotkeysProxyModel()
    {
    delete d; d=0;
    }


bool KHotkeysProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
    {
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
    }


// Convert index to ActionDataBase
KHotKeys::ActionDataBase *KHotkeysProxyModel::indexToActionDataBase( const QModelIndex &index ) const
    {
    return sourceModel()->indexToActionDataBase( mapToSource(index) );
    }


// Convert index to ActionDataGroup
KHotKeys::ActionDataGroup *KHotkeysProxyModel::indexToActionDataGroup( const QModelIndex &index ) const
    {
    return sourceModel()->indexToActionDataGroup( mapToSource(index) );
    }


// Return the source model
KHotkeysModel *KHotkeysProxyModel::sourceModel() const
    {
    return static_cast<KHotkeysModel*>( QSortFilterProxyModel::sourceModel() );
    }


#include "moc_hotkeys_proxy_model.cpp"
