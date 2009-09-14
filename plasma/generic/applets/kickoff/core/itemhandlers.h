/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

#ifndef ITEMHANDLERS_H
#define ITEMHANDLERS_H

#include <QtCore/QObject>

#include "core/kickoff_export.h"
#include "core/urlitemlauncher.h"

namespace Kickoff
{

class KICKOFF_EXPORT ServiceItemHandler : public UrlItemHandler
{
public:
    virtual bool openUrl(const KUrl& url);
};
class KICKOFF_EXPORT LeaveItemHandler : public QObject, public UrlItemHandler
{
    Q_OBJECT
public:
    virtual bool openUrl(const KUrl& url);

private Q_SLOTS:
    void runCommand();
    void logout();
    void lock();
    void switchUser();
    void saveSession();
    void standby();
    void suspendRAM();
    void suspendDisk();

private:
    QString m_logoutAction;
};

}

#endif // ITEMHANDLERS_H

