/**
  * kprivacymanager.h
  *
  * Copyright (c) 2003 Ralf Hoelzer <ralf@well.com>
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU Lesser General Public License as published
  *  by the Free Software Foundation; either version 2.1 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU Lesser General Public License for more details.
  *
  *  You should have received a copy of the GNU Lesser General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  */

#ifndef KPRIVACYMANAGER_H
#define KPRIVACYMANAGER_H

#include <qobject.h>

/**
@author Ralf Hoelzer
*/

class KPrivacyManager : public QObject
{
Q_OBJECT
public:
    KPrivacyManager();

    ~KPrivacyManager();

    bool clearAllCookies();
    bool clearSavedClipboardContents();
    bool clearRunCommandHistory();
    bool clearFormCompletion();
    bool clearWebHistory();
    bool clearWebCache();
    bool clearQuickStartMenu();
    bool clearRecentDocuments();
    bool clearFavIcons();

private:
    bool isApplicationRegistered(QString appName);
    bool m_error;

};

#endif
