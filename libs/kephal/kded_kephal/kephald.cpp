/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "kephald.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include "kephalservice.h"

K_PLUGIN_FACTORY(KephalDFactory,
        registerPlugin<KephalD>();
        )
K_EXPORT_PLUGIN(KephalDFactory("kephal"))

KephalD::KephalD(QObject* parent, const QList<QVariant>&)
: KDEDModule(parent)
{
    new KephalService(parent);
}

KephalD::~KephalD()
{
}

#include "kephald.moc"
