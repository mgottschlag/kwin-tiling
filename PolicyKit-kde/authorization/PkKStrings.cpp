/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PkKStrings.h"

#include <KLocale>

#include <KDebug>

PkKStrings::PkKStrings(QObject *parent)
        : QObject(parent)
{
}

PkKStrings::~PkKStrings()
{
}

QString PkKStrings::getPolKitResult(PolKitResult result)
{
    switch (result) {
    case POLKIT_RESULT_NO :
        return i18nc("Negative Result", "No");

    case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_ONE_SHOT :
        return i18n("Admin Authentication (one shot)");
    case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH :
        return i18n("Admin Authentication");
    case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_SESSION :
        return i18n("Admin Authentication (keep session)");
    case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_ALWAYS :
        return i18n("Admin Authentication (keep indefinitely)");

    case POLKIT_RESULT_ONLY_VIA_SELF_AUTH_ONE_SHOT :
        return i18n("Authentication (one shot)");
    case POLKIT_RESULT_ONLY_VIA_SELF_AUTH :
        return i18n("Authentication");
    case POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_SESSION :
        return i18n("Authentication (keep session)");
    case POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_ALWAYS :
        return i18n("Authentication (keep indefinitely)");

    case POLKIT_RESULT_YES :
        return i18nc("Positive Result", "Yes");
    default :
        kDebug() << "result unrecognised: " << result;
        return QString();
    }
}

#include "PkKStrings.moc"
