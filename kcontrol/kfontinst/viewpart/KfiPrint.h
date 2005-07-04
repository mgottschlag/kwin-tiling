#ifndef __PRINT_H__
#define __PRINT_H__

////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Print
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 14/05/2005
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

class QStringList;
class QString;
class QWidget;

namespace KFI
{

class CFcEngine;

namespace Print
{
extern void printItems(const QStringList &items, int size, QWidget *parent, CFcEngine &engine);
extern bool printable(const QString &mime);
}

}

#endif
