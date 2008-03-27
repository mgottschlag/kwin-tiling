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
#ifndef KHOTKEYSPROXYMODEL_H
#define KHOTKEYSPROXYMODEL_H

#include "libkhotkeysfwd.h"

#include <QtGui/QSortFilterProxyModel>



class KHotkeysProxyModelPrivate;
class KHotkeysModel;


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KHotkeysProxyModel : public QSortFilterProxyModel
    {
    Q_OBJECT

    public:

        /**
         * Default constructor
         */
        KHotkeysProxyModel( QObject *parent );

        /**
         * Destructor
         */
        virtual ~KHotkeysProxyModel();

        /**
         * Returns true if the item in column @a column should be included in
         * the model.
         */
        bool filterAcceptsRow( int source_column, const QModelIndex &source_parent ) const;

        /**
         * Get the KHotKeys::ActionDataBase behind the index.
         */
        KHotKeys::ActionDataBase *indexToActionDataBase( const QModelIndex &index ) const;

        /**
         * Get the KHotKeys::ActionDataBase behind the index or 0.
         *
         * Getting 0 doesn't mean the index is invalid. It means you provided a
         * action object.
         */
        KHotKeys::ActionDataGroup *indexToActionDataGroup( const QModelIndex &index ) const;

        /**
         * Return the source model. Casted to the correct class.
         */
        KHotkeysModel *sourceModel() const;

  private:

        //! Implementation details
        KHotkeysProxyModelPrivate *d;
};

#endif /* #ifndef KHOTKEYSPROXYMODEL_HPP */
