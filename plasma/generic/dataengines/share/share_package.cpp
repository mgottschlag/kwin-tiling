/***************************************************************************
 *   Copyright 2010 Artur Duque de Souza <asouza@kde.org>                  *
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

#include "plasma/applet.h"
#include "plasma/package.h"

#include "share_package.h"


SharePackage::SharePackage(QObject *parent, QVariantList args)
    : Plasma::PackageStructure(parent, "Plasma/ShareProvider")
{
    Q_UNUSED(args)

    addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    QStringList mimetypes;
    mimetypes << "text/*";
    setMimetypes( "scripts", mimetypes );

    addFileDefinition("mainscript", "code/main.js", i18n("Main Script File"));
    setDefaultPackageRoot("plasma/shareprovider/");
    setServicePrefix("plasma-share-");
}

