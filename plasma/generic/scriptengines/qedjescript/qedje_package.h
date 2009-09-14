/***************************************************************************
 *   Copyright (C) 2008 by Artur Duque de Souza <morpheuz@gmail.com>       *
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

#ifndef QEDJE_PACKAGE_H
#define QEDJE_PACKAGE_H

#include <Plasma/Package>
#include <Plasma/PackageStructure>
#include <Plasma/PackageMetadata>

class QEdjePackage : public Plasma::PackageStructure
{
    Q_OBJECT

public:
    QEdjePackage(QObject *parent, const QVariantList &args);

    virtual ~QEdjePackage();
    virtual bool installPackage(const QString &archivePath,
                                const QString &packageRoot);
};

#endif
