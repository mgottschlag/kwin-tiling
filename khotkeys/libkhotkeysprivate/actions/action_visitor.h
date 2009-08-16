#ifndef ACTION_VISITOR_H
#define ACTION_VISITOR_H
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

class Action;
class ActionList;
class ActivateWindowAction;
class CommandUrlAction;
class DBusAction;
class KeyboardInputAction;
class MenuEntryAction;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ActionVisitor
    {
public:
    ActionVisitor();
    virtual ~ActionVisitor();

private:

    virtual void visit(Action*)= 0
    virtual void visit(ActionList*)= 0
    virtual void visit(ActivateWindowAction*)= 0
    virtual void visit(CommandUrlAction*)= 0
    virtual void visit(DBusAction*)= 0
    virtual void visit(KeyboardInputAction*)= 0
    virtual void visit(MenuEntryAction*)= 0

    virtual void visit(Action*) const= 0
    virtual void visit(ActionList*) const= 0
    virtual void visit(ActivateWindowAction*) const= 0
    virtual void visit(CommandUrlAction*) const= 0
    virtual void visit(DBusAction*) const= 0
    virtual void visit(KeyboardInputAction*) const= 0
    virtual void visit(MenuEntryAction*) const= 0
    }; // ActionVisitor


} // namespace KHotKeys

#endif /* ACTION_VISITOR_H */

