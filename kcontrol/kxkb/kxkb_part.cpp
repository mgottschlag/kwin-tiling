/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KPluginFactory>
#include <KPluginLoader>

#include "kxkb_part.h"

K_PLUGIN_FACTORY(KxkbPartFactory, registerPlugin<KxkbPart>();)
K_EXPORT_PLUGIN(KxkbPartFactory("kfontview"))

KxkbPart::KxkbPart( QWidget* parentWidget,
               QObject* parent,
               const QList<QVariant>& args )
  : KParts::Part(parent)
{
}

bool 
KxkbPart::setLayout(const QString& layoutPair)
{
}
