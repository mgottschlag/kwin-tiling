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
#ifndef SHORTCUTACTIONS_H
#define SHORTCUTACTIONS_H


#include <KDE/KActionCollection>

#include <X11/X.h>


class KAction;
class KShortcut;

namespace KHotKeys {

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ShortcutsHandler : public QObject
    {
    Q_OBJECT

public:

    enum HandlerType {
        Active,             //!< Create real actions
        Configuration };    //!< Create configuration actions ( not active )

    /**
     * Default constructor
     *
     * \param Should 
     */
    ShortcutsHandler( HandlerType type = Active, QObject *parent = 0 );

    /**
     * Destructor
     */
    ~ShortcutsHandler();

    /**
     * Create a action.
     *
     * The action stays in the ownership of this class. Do not delete.
     *
     * \param id        Persistent id for the action
     * \param name      Name for the action. Is used in the global shortcut
     *                  configuration dialog
     * \param shortcut  Shortcut that triggers the action
     *
     * \return The new action or 0 if an error occured.
     *
     * \see KAction::registerGlobalShortcut()
     */
    KAction *addAction(
        const QString &id,
        const QString &text,
        const KShortcut &shortcut );

    /**
     * Remove a action from the collection.
     *
     * \param id        Persistent id for the action
     *
     * \return The action or 0 if not found.
     */
    QAction *getAction( const QString &id );

    /**
     * Remove a action from the collection.
     *
     * \param id        Persistent id for the action
     *
     * \return true if the action was removed.
     */
    bool removeAction( const QString &id );

    /**
     * From Kbd.
     *
     * \warning Does nothing, returns false
     */
    bool send_macro_key( const QString& key, Window window_P );

private:

    HandlerType _type;

    KActionCollection *_actions;

};

} // namespace KHotKeys

#endif /* #ifndef SHORTCUTACTIONS_H */
