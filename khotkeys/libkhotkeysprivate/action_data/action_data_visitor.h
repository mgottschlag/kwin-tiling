#ifndef ACTION_DATA_VISITOR_H
#define ACTION_DATA_VISITOR_H
/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

namespace KHotKeys {

class ActionData;
class ActionDataBase;
class ActionDataGroup;
class Generic_action_data;
class MenuEntryShortcutActionData;
class SimpleActionData;
template< typename T, typename A > class SimpleActionDataHelper;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ActionDataVisitor
    {
public:

    ActionDataVisitor();

    virtual ~ActionDataVisitor();

    virtual void visitActionDataBase(ActionDataBase *base);

    virtual void visitActionData(ActionData *base) = 0;

    virtual void visitActionDataGroup(ActionDataGroup *group) = 0;

    virtual void visitGenericActionData(Generic_action_data *data) = 0;

    virtual void visitMenuentryShortcutActionData(MenuEntryShortcutActionData *data) = 0;

    virtual void visitSimpleActionData(SimpleActionData *data) = 0;

    }; // ActionDataVisitor


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ActionDataConstVisitor
    {
public:

    ActionDataConstVisitor();

    virtual ~ActionDataConstVisitor();

    virtual void visitActionDataBase(const ActionDataBase *base);

    virtual void visitActionData(const ActionData *base) = 0;

    virtual void visitActionDataGroup(const ActionDataGroup *group) = 0;

    virtual void visitGenericActionData(const Generic_action_data *data) = 0;

    virtual void visitMenuentryShortcutActionData(const MenuEntryShortcutActionData *data) = 0;

    virtual void visitSimpleActionData(const SimpleActionData *data) = 0;

    template< typename T, typename A >
    KHotKeys::ActionDataBase *visitSimpleActionDataHelper(const KHotKeys::SimpleActionDataHelper<T, A> *object);

private:

    }; // ActionDataConstVisitor

} // KHotKeys

#endif /* ACTION_DATA_VISITOR_H */

