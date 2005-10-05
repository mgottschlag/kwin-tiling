#ifndef __FONTMAP_H__
#define __FONTMAP_H__

////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Fontmap
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 06/06/2003
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <qstring.h>
#include <qstringlist.h>
#include <q3ptrlist.h>

namespace KFI
{

class CFontEngine;

namespace Fontmap
{
    class CFile
    {
        private:

        struct TEntry
        {
            TEntry(const QString &fname) : filename(fname) {}

            QString     filename,
                        psName;
            QStringList entries;
        };

        public:

        CFile(const QString &dir);

        const QStringList * getEntries(const QString &fname);
        unsigned int        getLineCount() { return itsLineCount; }

        private:

        TEntry * findEntry(const QString &fname, bool isAlias=false);
        TEntry * getEntry(TEntry **current, const QString &fname, bool isAlias=false);

        private:

        QString           itsDir;
        Q3PtrList<TEntry> itsEntries;
        unsigned int      itsLineCount;
    };

    extern bool create(const QString &dir, CFontEngine &fe);
}

}

#endif
