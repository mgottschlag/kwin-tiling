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
        NameColumn,
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
     * \group Qt Model/View Framework methods
     */
    //@{
    QModelIndex index( int, int, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &index ) const;
    int columnCount( const QModelIndex &index ) const;
    QVariant headerData( int section, Qt::Orientation, int role = Qt::DisplayRole ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role );
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    //@}

    /**
     * \group Drag and Drop Support
     */
    //@{
    bool dropMimeData(
            const QMimeData *data
            ,Qt::DropAction action
            ,int row
            ,int column
            ,const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    //@}

    /**
     * Add a group as child of @a parent.
     *
     * @return the index for the new group
     */
    QModelIndex addGroup( const QModelIndex &parent );

    /**
     * Export the input actions into @a config.
     */
    void exportInputActions(
            const QModelIndex &index,
            KConfigBase &config,
            const QString& id,
            const KHotKeys::ActionState state,
            bool allowMerging);

    /**
     *Import the input actions from @a config.
     */
    void importInputActions(const QModelIndex &index, KConfigBase const &config);

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
     * Insert @a data as a child of @a parent.
     */
    QModelIndex insertActionData( KHotKeys::ActionDataBase *data, const QModelIndex &parent );

    /**
     * Load the settings from the file
     */
    void load();

    /**
     * Move @p element to @p newGroup at @position.
     *
     * @param element  move this element
     * @param newGroup to this group
     * @param position and put it at this position. default is last
     *
     * @return @c true if moved, @c false if not.
     */
    bool moveElement(
            KHotKeys::ActionDataBase *element
            ,KHotKeys::ActionDataGroup *newGroup
            ,int position = -1);

    /**
     * Remove @a count rows starting with @a row under @a parent.
     */
    bool removeRows( int row, int count, const QModelIndex &parent );

    /**
     * Save the settings to the file
     */
    void save();

    /**
     * Return the settings we handle
     */
    KHotKeys::Settings *settings();

    void emitChanged( KHotKeys::ActionDataBase *item );

  private:

    KHotKeys::Settings _settings;
    KHotKeys::ActionDataGroup *_actions;

};

#endif /* #ifndef KHOTKEYSMODEL_HPP */
