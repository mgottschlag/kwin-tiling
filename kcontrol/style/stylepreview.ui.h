/*
 * Style Preview Widget
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
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
 *
 ***************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#include <qobjectlist.h>

void StylePreview::init()
{
    // Ensure that the user can't toy with the child widgets.
    // Method borrowed from Qt's qtconfig.
    QObjectList* l = queryList("QWidget");
    QObjectListIt it(*l);
    QObject* obj;
    while ((obj = it.current()) != 0)
    {
        ++it;
        obj->installEventFilter(this);
        ((QWidget*)obj)->setFocusPolicy(NoFocus);
    }
    delete l;
}

bool StylePreview::eventFilter( QObject* /* obj */, QEvent* ev )
{
    switch( ev->type() )
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::Wheel:
        case QEvent::ContextMenu:
            return TRUE; // ignore
        default:
            break;
    }
    return FALSE;
}

// vim: set noet ts=4:
