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
#ifndef KHOTKEYSMODEL_H
#define KHOTKEYSMODEL_H

#include "libkhotkeysfwd.h"
#include "settings.h"

#include <QtCore/QAbstractItemModel>


/**
 */
class KHotkeysModel : public QAbstractItemModel
    {
    Q_OBJECT

    public:

    enum ItemType {
        Other                          //!< Some unknown action type
        ,ActionDataGroup               //!< A shortcut group
        ,SimpleActionData
    };

    enum Column {
        NameColumn = 0,
        EnabledColumn,
        IsGroupColumn,
        TypeColumn };

    /**
     * Default constructor
     *
     * @param 
     */
    KHotkeysModel( QObject *parent = 0 );

    /**
     * Destructor
     */
    virtual ~KHotkeysModel();

    /**
     * Standard methods required by Qt model/view framework
     */
    //@{
    QModelIndex index( int, int, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &index ) const;
    int columnCount( const QModelIndex &index ) const;
    QVariant headerData( int section, Qt::Orientation, int role = Qt::DisplayRole ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    //@}

    bool removeRows( int row, int count, const QModelIndex &parent );
    QModelIndex addGroup( const QModelIndex &parent );
    QModelIndex insertActionData( KHotKeys::ActionDataBase *data, const QModelIndex &parent );
    bool setData( const QModelIndex &index, const QVariant &value, int role );

    Qt::ItemFlags flags( const QModelIndex &index ) const;

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
     * Load the settings from the file
     */
    void load();

    /**
     * Save the settings to the file
     */
    void save();

    void emitChanged( KHotKeys::ActionDataBase *item );

  private:


    KHotKeys::Settings _settings;
    KHotKeys::ActionDataGroup *_actions;
};

#endif /* #ifndef KHOTKEYSMODEL_HPP */
