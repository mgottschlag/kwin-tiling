/*
* saverconfig.cpp
* Copyright 1997       Matthias Hoelzer
* Copyright 1996,1999,2002    Martin R. Jones
* Copyright 2004       Chris Howells
* Copyright 2007-2008  Benjamin Meyer <ben@meyerhome.net>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "saverconfig.h"

#include <KDesktopFile>
#include <KLocale>
#include <KConfigGroup>

SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(const QString &file)
{
    KDesktopFile config(file);
    const KConfigGroup group = config.desktopGroup();
#if 0
    if( !config.tryExec())
      return false;
#endif
    mExec = group.readPathEntry("Exec", QString());
    mName = group.readEntry("Name");
    QString categoryName = group.readEntry("X-KDE-Category");
    if(categoryName.isEmpty())
	mCategory = categoryName;
    else
        mCategory = i18nc("Screen saver category", // Must be same in CMakeFiles.txt
                     categoryName.toUtf8());

    if (config.hasActionGroup("Setup"))
    {
      mSetup = config.actionGroup("Setup").readPathEntry("Exec", QString());
    }

    if (config.hasActionGroup("InWindow"))
    {
      mSaver = config.actionGroup("InWindow").readPathEntry("Exec", QString());
    }

    int indx = file.lastIndexOf('/');
    if (indx >= 0) {
        mFile = file.mid(indx+1);
    }

    return !mSaver.isEmpty();
}
