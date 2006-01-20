/* This file is part of the KDE project
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef __kcustommenu_h__
#define __kcustommenu_h__

#include <QMenu>
#include <kservice.h>

/*
 * This class provides a Popup Menu with programs that can be executed and
 * that reads its configuration from a config file.
 */
class KCustomMenu : public QMenu
{
   Q_OBJECT
public:
   /**
    * Create a custome menu described by @p configfile.
    */
   KCustomMenu(const QString &configfile, QWidget *parent=0);

   /**
    * Destructor
    */
   ~KCustomMenu();
   
protected Q_SLOTS:
   void slotActivated(int id);

protected:      
   void insertMenuItem(KService::Ptr &s, int nId, int nIndex = -1);

private:
   class KCustomMenuPrivate;
   KCustomMenuPrivate *d;   
};


#endif
